#include <vob/aoe/physics/systems/physics_system.h>


// WIP vehicles / wheels
#include <bullet/BulletCollision/CollisionShapes/btCylinderShape.h>
#include <bullet/BulletCollision/CollisionShapes/btSphereShape.h>
#include <bullet/BulletDynamics/ConstraintSolver/btHingeConstraint.h>
#include <bullet/BulletDynamics/ConstraintSolver/btHinge2Constraint.h>
#include <vob/aoe/physics/debug_drawer.h>

#include "imgui.h"


namespace vob::aoeph
{
	physics_system::physics_system(aoeng::world_data_provider& a_wdp)
		: m_physicsWorldComponent{ a_wdp }
		, m_simulationTimeWorldComponent{ a_wdp }
		, m_imGuiContextWorldComponent{ a_wdp }
		, m_movingEntities{ a_wdp }
		, m_inputs{ a_wdp }
		, m_bindings{ a_wdp }
		, m_debugMeshWorldComponent{ a_wdp }
		, m_carControllerWorldComponent{ a_wdp }
		, m_carEntities{ a_wdp }
		, m_hingeCarEntities{ a_wdp }
	{
	}

	class wheel_ray_cast_callback : public btCollisionWorld::ClosestRayResultCallback
	{
	public:
		wheel_ray_cast_callback(glm::vec3 a_from, glm::vec3 a_to, btCollisionObject* a_car)
			: ClosestRayResultCallback(to_bt(a_from), to_bt(a_to))
			, m_car{ a_car }
		{}

		btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
		// btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace) override
		{
			if (rayResult.m_collisionObject == m_car)
			{
				return 1.0f;
			}

			return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
		}

	private:
		btCollisionObject* m_car;
	};

	class wheel_convex_sweep_callback : public btCollisionWorld::ClosestConvexResultCallback
	{
	public:
		wheel_convex_sweep_callback(glm::vec3 a_from, glm::vec3 a_to, btCollisionObject* a_car)
			: ClosestConvexResultCallback(to_bt(a_from), to_bt(a_to))
			, m_car{ a_car }
		{}

		btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace) override
		{
			if (convexResult.m_hitCollisionObject == m_car)
			{
				return 1.0f;
			}

			return ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
		}

	private:
		btCollisionObject* m_car;
	};

	class wheel_contact_callback : public btCollisionWorld::ContactResultCallback
	{
	public:
		btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override
		{
			static bool stop = false;
			if (stop)
			{
				__debugbreak();
			}
			return 1.0f;
		}
	};

	void debug(char const* name, float value)
	{
		ImGui::Text(name);
		ImGui::SameLine(150);
		ImGui::Text("%.3f", value);
	}

	void debug(char const* name, std::int32_t value)
	{
		ImGui::Text(name);
		ImGui::SameLine(150);
		ImGui::Text("%i", value);
	}

	void debug(char const* name, glm::vec3 const& value)
	{
		ImGui::Text(name);
		ImGui::SameLine(150);
		ImGui::Text("%.3f, %.3f, %.3f", value.x, value.y, value.z);
	}

	void debug(char const* name, bool value)
	{
		ImGui::Text(name);
		ImGui::SameLine(150);
		ImGui::BeginDisabled();
		ImGui::Checkbox("##", &value);
		ImGui::EndDisabled();
	}

	void physics_system::update() const
	{
		auto const dt = m_simulationTimeWorldComponent->m_elapsedTime.get_value();
		auto& physicsWorld = m_physicsWorldComponent->m_world.get();
		physicsWorld.stepSimulation(dt, 10, 0.01f);

		// WIP vehicles / wheels
		bool const resetPosition = m_inputs->mouse.buttons[aoein::mouse::button::M3].is_pressed();
		float extraSuspensionLength = 1.0f;
		float extraSpringStrength = 1.0f;
		float extraDamperStrength = 1.0f;
		bool const rmb = m_inputs->mouse.buttons[aoein::mouse::button::Right].is_pressed();
		bool const shift = m_inputs->keyboard.keys[aoein::keyboard::key::LShift].is_pressed();
		bool const control = m_inputs->keyboard.keys[aoein::keyboard::key::LControl].is_pressed();
		bool const fwd = m_inputs->keyboard.keys[aoein::keyboard::key::E].is_pressed() && !rmb;
		bool const bwd = m_inputs->keyboard.keys[aoein::keyboard::key::D].is_pressed() && !rmb;
		bool const lft = m_inputs->keyboard.keys[aoein::keyboard::key::S].is_pressed() && !rmb;
		bool const rgt = m_inputs->keyboard.keys[aoein::keyboard::key::F].is_pressed() && !rmb;

		auto& axes = m_bindings->axes;
		auto& switches = m_bindings->switches;
		float const turn = -axes[m_carControllerWorldComponent->m_turn]->get_value();
		// float const engine = axes[m_carControllerWorldComponent->m_engine]->get_value();

		bool const forward = switches[m_carControllerWorldComponent->m_forward]->is_pressed();
		bool const reverse = switches[m_carControllerWorldComponent->m_reverse]->is_pressed();

		bool const respawn = switches[m_carControllerWorldComponent->m_respawn]->was_pressed();

		if (shift && control)
		{
			if (m_inputs->mouse.buttons[aoein::mouse::button::ScrollUp].was_pressed())
			{
				extraDamperStrength *= 1.1f;
			}
			else if (m_inputs->mouse.buttons[aoein::mouse::button::ScrollDown].was_pressed())
			{
				extraDamperStrength /= 1.1f;
			}
		}
		else if (shift)
		{
			if (m_inputs->mouse.buttons[aoein::mouse::button::ScrollUp].was_pressed())
			{
				extraSpringStrength *= 1.1f;
			}
			else if (m_inputs->mouse.buttons[aoein::mouse::button::ScrollDown].was_pressed())
			{
				extraSpringStrength /= 1.1f;
			}
		}
		else if (control)
		{
			if (m_inputs->mouse.buttons[aoein::mouse::button::ScrollUp].was_pressed())
			{
				extraSuspensionLength *= 1.1f;
			}
			else if (m_inputs->mouse.buttons[aoein::mouse::button::ScrollDown].was_pressed())
			{
				extraSuspensionLength /= 1.1f;
			}
		}

		const bool isImGuiWindowOpen = ImGui::Begin("Physics");
		/* hinge2vehicle for (auto [entity, position, rotation, carController] : m_hingeCarEntities.get().each())
		{
			if (respawn)
			{
				auto const defaultTransform = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.0f, 8.0f, 0.0f });
				carController.m_chassisRigidBody->setWorldTransform(to_bt(defaultTransform));
				carController.m_chassisRigidBody->setLinearVelocity(to_bt(glm::vec3{ 0.0f }));
				carController.m_chassisRigidBody->setAngularVelocity(to_bt(glm::vec3{ 0.0f }));
				carController.m_chassisRigidBody->getMotionState()->setWorldTransform(to_bt(defaultTransform));
				carController.m_chassisRigidBody->clearForces();

				for (auto& wheelRigidBody : carController.m_wheelRigidBodies)
				{
					wheelRigidBody.first->setWorldTransform(to_bt(glm::translate(defaultTransform, wheelRigidBody.second)));
					wheelRigidBody.first->setLinearVelocity(btVector3(0, 0, 0));
					wheelRigidBody.first->setAngularVelocity(btVector3(0, 0, 0));
					wheelRigidBody.first->getMotionState()->setWorldTransform(to_bt(glm::translate(defaultTransform, wheelRigidBody.second)));
					wheelRigidBody.first->clearForces();
				}
			}

			if (dt > 0.0f)
			{
				for (auto& wheelHinge : carController.m_wheelHinges)
				{
					wheelHinge->setTargetVelocity(3, engine);
				}
			}
		}*/

		static float sBaseFriction = 1.0f; // 1.1 = rubber?
		debug_drawer debugDrawer{ *m_debugMeshWorldComponent };
		auto drawWheel = [&debugDrawer](glm::mat4 const& transform)
		{
			constexpr int k_hSlices = 12;
			constexpr int k_vSlices = 24;
			for (int i = 0; i < k_hSlices; ++i)
			{
				auto const a = std::numbers::pi_v<float> *((1.0f * i) / k_hSlices - 0.5f);
				auto const r = std::cos(a);
				auto const y = std::sin(a);
				for (int j = 0; j < k_vSlices; ++j)
				{
					auto const b = 2.0f * std::numbers::pi_v<float> *(1.0f * j) / k_vSlices;
					auto const c = 2.0f * std::numbers::pi_v<float> *(1.0f * (j + 1)) / k_vSlices;
					auto const xb = r * std::cos(b);
					auto const zb = r * std::sin(b);
					auto const xc = r * std::cos(c);
					auto const zc = r * std::sin(c);

					auto const pb = glm::vec3{ transform * glm::vec4(xb, y, zb, 1.0f) };
					auto const pc = glm::vec3{ transform * glm::vec4(xc, y, zc, 1.0f) };
					debugDrawer.drawLine(to_bt(pb), to_bt(pc), to_bt(aoegl::k_yellow));
				}
			}
			for (int j = 0; j < k_vSlices; ++j)
			{
				auto const a = 2.0f * std::numbers::pi_v<float> *(1.0f * j) / k_vSlices;
				auto const xa = std::cos(a);
				auto const za = std::sin(a);

				for (int i = 0; i < k_hSlices; ++i)
				{
					auto const b = std::numbers::pi_v<float> *((1.0f * i) / k_hSlices - 0.5f);
					auto const rb = std::cos(b);
					auto const yb = std::sin(b);
					auto const c = std::numbers::pi_v<float> *((1.0f * (i + 1)) / k_hSlices - 0.5f);
					auto const rc = std::cos(c);
					auto const yc = std::sin(c);

					auto const pb = glm::vec3{ transform * glm::vec4(xa * rb, yb, za * rb, 1.0f) };
					auto const pc = glm::vec3{ transform * glm::vec4(xa * rc, yc, za * rc, 1.0f) };
					debugDrawer.drawLine(to_bt(pb), to_bt(pc), to_bt(aoegl::k_yellow));
				}
			}
		};

		for (auto [entity, position, rotation, carController, rigidBody] : m_carEntities.get().each())
		{
			bool isGrounded = false;

			if (respawn)
			{
				auto const defaultTransform = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.0f, 8.0f, 0.0f });
				rigidBody.m_instance->setWorldTransform(to_bt(defaultTransform));
				rigidBody.m_instance->setLinearVelocity(to_bt(glm::vec3{ 0.0f }));
				rigidBody.m_instance->setAngularVelocity(to_bt(glm::vec3{ 0.0f }));
				rigidBody.m_instance->getMotionState()->setWorldTransform(to_bt(defaultTransform));
				rigidBody.m_instance->clearForces();
			}
			if (resetPosition)
			{
				if (control && shift)
				{
					rigidBody.m_instance->setLinearVelocity(to_bt(glm::vec3{ 0.0f, 0.0f, 0.0f }));
					rigidBody.m_instance->setAngularVelocity(to_bt(glm::vec3{ 0.0f, 0.0f, 0.0f }));
				}
				else if (control)
				{
					rigidBody.m_instance->setAngularVelocity(to_bt(glm::vec3{ 0.0f, 1.0f, 0.0f }));
				}
				else if (shift)
				{
					rigidBody.m_instance->setLinearVelocity(to_bt(glm::vec3{ 0.0f, 0.0f, -1.0f }));
				}
				else
				{
					auto const defaultTransform = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.0f, 8.0f, 0.0f });
					rigidBody.m_instance->setWorldTransform(to_bt(defaultTransform));
					rigidBody.m_instance->setLinearVelocity(to_bt(glm::vec3{ 0.0f }));
					rigidBody.m_instance->setAngularVelocity(to_bt(glm::vec3{ 0.0f }));
					rigidBody.m_instance->getMotionState()->setWorldTransform(to_bt(defaultTransform));
					rigidBody.m_instance->clearForces();
				}
			}

			auto const transform = aoest::combine(position, rotation);

			const glm::vec3 velocity = to_glm(rigidBody.m_instance->getLinearVelocity());
			const float speed = glm::length(velocity);
			const float downForceStrength = speed > 13.88 ? 0.08f : 0.0f;
			// TODO: not down force if vertical speed
			const glm::vec3 downLocalForce = glm::vec3{ 0.0f, -downForceStrength * speed * 10, 0.0f};
			const glm::vec3 downForce = rotation * downLocalForce;

			const glm::vec3 forwardDir = rotation * glm::vec3{ 0.0f, 0.0f, -1.0f };
			const float frontSpeed = glm::dot(velocity, forwardDir);
			const bool isGoingForward = glm::dot(velocity, forwardDir) >= 0.0f;
			const bool isBraking = (isGoingForward && reverse) || (!isGoingForward && forward);
			
			auto const downForceEndTransform = glm::translate(transform, downForce);
			m_debugMeshWorldComponent->add_line(
				aoest::get_position(transform),
				aoest::get_position(downForceEndTransform),
				aoegl::k_blue);

			if (dt > 0.0f)
			{
				rigidBody.m_instance->applyCentralForce(to_bt(downForce));
			}

			

			auto const invTransform = glm::inverse(transform);
			int i = 0;

			static float k_x = 0.182f;
			static float k_y = 0.364f;
			static float k_z = 0.364f;
			ImGui::SliderFloat("x", &k_x, 0.182f, 1.0f);
			ImGui::SliderFloat("y", &k_y, 0.182f, 1.0f);
			ImGui::SliderFloat("z", &k_z, 0.182f, 1.0f);

			for (auto& wheel : carController.m_wheels)
			{
				auto wheelRot = glm::quat{ 0.0f, 0.0f, 0.0f, 1.0f };
				auto maxAngleDeg = wheel.m_rotationFactor * std::lerp(glm::radians(40.0f), glm::radians(25.0f), std::clamp(speed * 3.6f, 0.0f, 200.0f) / 200.0f);
				if (!rmb)
				{
					wheelRot = glm::rotate(wheelRot, turn * maxAngleDeg, -glm::vec3{ 0.0f, 1.0f, 0.0f });
				}

				wheel.m_springStrength *= extraSpringStrength;
				wheel.m_damperStrength *= extraDamperStrength;
				wheel.m_suspensionLength *= extraSuspensionLength;

				if (dt > 0.0f)
				{
					auto const attachmentPosition = aoest::get_position(
						glm::translate(transform, wheel.m_attachmentRelativePosition));
					auto const extendedPosition = attachmentPosition +
						rotation * glm::vec3{ 0.0f, -wheel.m_suspensionLength - wheel.m_radius, 0.0f };
					// wheel_ray_cast_callback closestHit(attachmentPosition, extendedPosition, rigidBody.m_instance.get());
					btCylinderShapeX wheelShape{ btVector3{wheel.m_width, wheel.m_radius, wheel.m_radius} };
					btSphereShape wheelShape2{ 1.0f };
					wheelShape2.setLocalScaling(btVector3(k_y, k_z, k_x));
					auto const sourceTr = aoest::combine(attachmentPosition, rotation);
					auto const targetTr = aoest::combine(extendedPosition, rotation);

					wheel_convex_sweep_callback closestHit(attachmentPosition, extendedPosition, rigidBody.m_instance.get());
					physicsWorld.convexSweepTest(&wheelShape2, to_bt(sourceTr), to_bt(targetTr), closestHit);

					btRigidBody wheelRb(0.0f, nullptr, &wheelShape);
					wheelRb.setWorldTransform(to_bt(aoest::combine(attachmentPosition, rotation)));
					wheel_contact_callback contactCb;
					physicsWorld.contactTest(&wheelRb, contactCb);
					//physicsWorld.rayTest(to_bt(attachmentPosition), to_bt(extendedPosition), closestHit);
					if (closestHit.hasHit())
					{
						auto cleanup = [](glm::vec4 const& v) { return glm::vec3{ v / v.w }; };
						auto const contactPosition = to_glm(closestHit.m_hitPointWorld);
						auto const contactNormal = to_glm(closestHit.m_hitNormalWorld);
						auto const friction = closestHit.m_hitCollisionObject->getFriction() * sBaseFriction;
						wheel.update_contact_point(1.0f - closestHit.m_closestHitFraction, contactPosition, contactNormal, friction, dt);

						auto const suspensionStrength = wheel.calculate_suspension_strength();
						auto const suspensionForce = suspensionStrength * (rotation * glm::vec3{ 0.0f, 1.0f, 0.0f });

						auto const forceRelPoint =
							rotation * (cleanup(invTransform * glm::vec4{ wheel.m_contactPoint->m_position, 1.0f }) + glm::vec3{ 0.0f, wheel.m_radius, 0.0f });

						rigidBody.m_instance->applyForce(to_bt(suspensionForce), to_bt(forceRelPoint));

						// TODO: should account for wheel's friction
						auto const contactVelocity = to_glm(rigidBody.m_instance->getVelocityInLocalPoint(to_bt(rotation * wheel.m_attachmentRelativePosition)));
						auto const sideDir = rotation * wheelRot * glm::vec3{ 1.0f, 0.0f, 0.0f };
						auto const lateralSlideSpeed = glm::dot(contactVelocity, sideDir);
						auto const lateralFrictionForce = -sideDir * lateralSlideSpeed * friction / rigidBody.m_instance->getInvMass();
						rigidBody.m_instance->applyForce(to_bt(lateralFrictionForce), to_bt(forceRelPoint));

					}
					else
					{
						wheel.m_contactPoint.reset();
					}
				}


				if (wheel.m_contactPoint.has_value())
				{
					isGrounded = true;
					auto const attachmentPosition = aoest::get_position(
						glm::translate(transform, wheel.m_attachmentRelativePosition));
					auto const contactPosition = wheel.m_contactPoint->m_position;
					auto const contactNormal = rotation * glm::vec3{ 0.0f, 1.0f, 0.0f };
					auto const suspensionStrength = wheel.calculate_suspension_strength();
					m_debugMeshWorldComponent->add_line(contactPosition, contactPosition + contactNormal * suspensionStrength * 0.002f, aoegl::k_red);

					auto const wheelPosition = attachmentPosition
						- (contactNormal * (
							(1 - wheel.m_contactPoint->m_compressionRatio) * (wheel.m_suspensionLength + wheel.m_radius)));
					// auto const wheelPosition = contactPosition + contactNormal * wheel.m_radius;
					auto const wheelTransform = aoest::combine(wheelPosition, rotation) * aoest::combine(glm::vec3{ 0.0f }, wheelRot);
					//debugDrawer.drawCylinder(wheel.m_radius, wheel.m_width * 0.5f, 0, to_bt(wheelTransform), to_bt(aoegl::k_green));
					{
						drawWheel(glm::scale(wheelTransform, glm::vec3(k_x, k_y, k_z)));
					}
					debugDrawer.drawSphere(to_bt(contactPosition), 0.05f, to_bt(aoegl::k_red));

					// slide friction
					auto cleanup = [](glm::vec4 const& v) { return glm::vec3{ v / v.w }; };
					auto const contactVelocity = to_glm(rigidBody.m_instance->getVelocityInLocalPoint(to_bt(rotation * wheel.m_attachmentRelativePosition)));

					auto const sideDir = rotation * wheelRot * glm::vec3{ 1.0f, 0.0f, 0.0f };
					auto const lateralSlideSpeed = glm::dot(contactVelocity, sideDir);
					auto const lateralFrictionForce = -sideDir * lateralSlideSpeed * wheel.m_contactPoint->m_friction;
					auto const kh = glm::vec3{ 0.0, 0.1, 0.0 };
					m_debugMeshWorldComponent->add_line(contactPosition + kh, contactPosition + lateralFrictionForce + kh, aoegl::k_purple);
				}
				else
				{
					auto const attachmentTransform = glm::translate(transform, wheel.m_attachmentRelativePosition);
					auto const extendedTransform = glm::translate(attachmentTransform, glm::vec3{ 0.0f, -wheel.m_suspensionLength- wheel.m_radius, 0.0f }) * aoest::combine(glm::vec3{ 0.0f }, wheelRot);
					m_debugMeshWorldComponent->add_line(
						aoest::get_position(attachmentTransform),
						aoest::get_position(extendedTransform),
						aoegl::k_blue);
					{
						drawWheel(glm::scale(extendedTransform, glm::vec3(k_x, k_y, k_z)));
					}

					//debugDrawer.drawCylinder(wheel.m_radius, wheel.m_width * 0.5f, 0, to_bt(extendedTransform), to_bt(aoegl::k_yellow));
				}

				++i;
			}

			const float engine = forward ? (reverse ? 0.0f : 1.0f) : (reverse ? -1.0f : 0.0f);
			if (isGrounded)
			{
				// should be done on wheel, if touching ground
				rigidBody.m_instance->applyCentralForce(to_bt(rotation * glm::vec3{ 0.0f, 0.0f, -10000.0f * engine }));

			}
			else if (engine < -0.1f)
			{
				rigidBody.m_instance->setAngularVelocity(btVector3{ 0.0f, 0.0f, 0.0f });
			}

			if (isImGuiWindowOpen)
			{
				debug("Input Steer", turn);
				debug("Input Is Braking", isBraking);
				debug("Input Gas Pedal", forward);
				debug("Input Brake Pedal", reverse);
				debug("Is Ground Contact", isGrounded);
				// debug("Is Wheels Burning", ...);
				// debug("Ground Dist", ...);
				// debug("Cur Gear", ...);
				debug("World Vel", velocity);
				debug("Front Speed", frontSpeed);
				// debug("Air Brake Normed", ...);
				// debug("Spoiler Open Normed", ...);
				debug("Wings Open Normed", downForceStrength);
				// debug("Is Top Contact", ...);

				debug("FL Steer Angle", turn * carController.m_wheels[1].m_rotationFactor * std::lerp(glm::radians(40.0f), glm::radians(25.0f), std::clamp(speed * 3.6f, 0.0f, 200.0f) / 200.0f));
				debug("FR Steer Angle", turn * carController.m_wheels[3].m_rotationFactor * std::lerp(glm::radians(40.0f), glm::radians(25.0f), std::clamp(speed * 3.6f, 0.0f, 200.0f) / 200.0f));
				debug("RL Steer Angle", turn * carController.m_wheels[0].m_rotationFactor * std::lerp(glm::radians(40.0f), glm::radians(25.0f), std::clamp(speed * 3.6f, 0.0f, 200.0f) / 200.0f));
				debug("RR Steer Angle", turn * carController.m_wheels[2].m_rotationFactor * std::lerp(glm::radians(40.0f), glm::radians(25.0f), std::clamp(speed * 3.6f, 0.0f, 200.0f) / 200.0f));
				debug("FL Damper Len", carController.m_wheels[1].m_suspensionLength * (1.0f - carController.m_wheels[1].m_contactPoint.value_or(contact_point{ 0.0f }).m_compressionRatio));
				debug("FR Damper Len", carController.m_wheels[3].m_suspensionLength * (1.0f - carController.m_wheels[3].m_contactPoint.value_or(contact_point{ 0.0f }).m_compressionRatio));
				debug("RL Damper Len", carController.m_wheels[0].m_suspensionLength * (1.0f - carController.m_wheels[0].m_contactPoint.value_or(contact_point{ 0.0f }).m_compressionRatio));
				debug("RR Damper Len", carController.m_wheels[2].m_suspensionLength * (1.0f - carController.m_wheels[2].m_contactPoint.value_or(contact_point{ 0.0f }).m_compressionRatio));

				ImGui::Separator();
				debug("Max Angle", std::lerp(glm::radians(40.0f), glm::radians(25.0f), std::clamp(speed * 3.6f, 0.0f, 200.0f) / 200.0f));
				debug("Front Speed (kph)", static_cast<int>(std::round(frontSpeed * 3.6f)));

				/*ImGui::Text("Down Force");
				ImGui::SameLine(150);
				ImGui::Text("%f, %f, %f", downLocalForce.x, downLocalForce.y, downLocalForce.z);*/
			}

			debug_drawer debugDrawer(*m_debugMeshWorldComponent);
			
			auto box = static_cast<btBoxShape const*>(rigidBody.m_collisionShape.get());
			debugDrawer.drawBox(-box->getHalfExtentsWithMargin(), box->getHalfExtentsWithMargin(), to_bt(transform), to_bt(aoegl::k_white));
			float springStrength = carController.m_wheels[0].m_springStrength;
			float damperStrength = carController.m_wheels[0].m_damperStrength;
			float attachmentHeight = carController.m_wheels[0].m_attachmentRelativePosition.y;
			ImGui::SliderFloat("String Strength", &springStrength, 0.0f, 25000.0f);
			ImGui::SliderFloat("Damper Strength", &damperStrength, 0.0f, 4000.0f);
			ImGui::SliderFloat("Attachment Height", &attachmentHeight, -1.0f, 1.0f);
			carController.m_wheels[0].m_springStrength = springStrength;
			carController.m_wheels[1].m_springStrength = springStrength;
			carController.m_wheels[2].m_springStrength = springStrength;
			carController.m_wheels[3].m_springStrength = springStrength;
			carController.m_wheels[0].m_damperStrength = damperStrength;
			carController.m_wheels[1].m_damperStrength = damperStrength;
			carController.m_wheels[2].m_damperStrength = damperStrength;
			carController.m_wheels[3].m_damperStrength = damperStrength;
			carController.m_wheels[0].m_attachmentRelativePosition.y = attachmentHeight;
			carController.m_wheels[1].m_attachmentRelativePosition.y = attachmentHeight;
			carController.m_wheels[2].m_attachmentRelativePosition.y = attachmentHeight;
			carController.m_wheels[3].m_attachmentRelativePosition.y = attachmentHeight;
			ImGui::SliderFloat("Base Friction", &sBaseFriction, 0.0f, 2.0f);

			rigidBody.m_instance->activate(true);
		}

		ImGui::End();
	}

	/*void physics_system::on_spawn(aoeng::registry& a_registry, aoeng::entity a_entity) const
	{
		auto entity = aoeng::entity_handle{ a_registry, a_entity };
		auto& dynamicsWorld = m_physicsWorldComponent->m_world.get();
		
		// rigidbody
		auto [position, rotation, collider] = a_registry.try_get<aoest::position, aoest::rotation, collider_component>(a_entity);
		if (position != nullptr && rotation != nullptr && collider != nullptr)
		{
			ignorable_assert(collider->m_shape != nullptr && "collider doesn't have a shape.");
			ignorable_assert(collider->m_material != nullptr && "collider doesn't have a material.");

			auto& motionState = create_motion_state(entity, *position, *rotation, *collider);
			auto& rigidbody = create_rigidbody(entity, *collider, motionState);

			dynamicsWorld.addRigidBody(rigidbody.m_instance.get());
			return;
		}

		auto pawn = a_registry.try_get<pawn_component>(a_entity);
		if (position != nullptr && rotation != nullptr && pawn != nullptr)
		{
			auto& pawnController = entity.emplace<pawn_controller_component>();
			pawnController.m_instance = std::make_shared<pawn_controller>();
			pawnController.m_instance->m_position = to_bt(*position);
			dynamicsWorld.addAction(pawnController.m_instance.get());
		}
	}

	void physics_system::on_despawn(aoeng::registry& a_registry, aoeng::entity a_entity) const
	{
		auto& dynamicsWorld = m_physicsWorldComponent->m_world.get();
		
		auto rigidbody = a_registry.try_get<rigidbody_component>(a_entity);
		if (rigidbody != nullptr)
		{
			dynamicsWorld.removeRigidBody(rigidbody->m_instance.get());
		}

		auto pawnController = a_registry.try_get<pawn_controller_component>(a_entity);
		if (pawnController != nullptr)
		{
			dynamicsWorld.removeAction(pawnController->m_instance.get());
		}
	}

	physics_system::motion_state_component& physics_system::create_motion_state(
		aoeng::entity_handle a_entity,
		aoest::position const& a_position,
		aoest::rotation const& a_rotation,
		collider_component const& a_collider) const
	{
		auto const btLocalTransform = to_bt(aoest::combine(a_position, a_rotation));
		auto const btOffsetTransform = to_bt(glm::translate(glm::mat4{ 1.0f }, a_collider.m_offset));

#pragma message(VOB_MISTD_TODO "support custom allocator.")
		return a_entity.emplace<motion_state_component>(
			std::make_shared<btDefaultMotionState>(btLocalTransform, btOffsetTransform));
	}

	physics_system::rigidbody_component& physics_system::create_rigidbody(
		vob::aoeng::entity_handle a_entity,
		vob::aoeph::collider_component const& a_collider,
		motion_state_component& a_motionState) const
	{
		auto* shape = a_collider.m_shape != nullptr ? a_collider.m_shape.get() : &k_defaultShape;
		auto const& material = a_collider.m_material != nullptr ? *a_collider.m_material : k_defaultMaterial;

		auto const mass = a_collider.m_mass;
		btVector3 inertia{ 0.0, 0.0, 0.0 };
		if (mass != btScalar{ 0.0 })
		{
			shape->calculateLocalInertia(mass, inertia);
		}

#pragma message(VOB_MISTD_HACK "for some reason shape cannot be const, but nobody will modify it.")
		auto rigidbody = std::make_shared<btRigidBody>(
			mass, a_motionState.m_instance.get(), const_cast<btCollisionShape*>(shape), inertia);

		rigidbody->setGravity(btVector3{ btScalar{ 0.0 }, btScalar{ -k_g }, btScalar{ 0.0 } });
		rigidbody->setLinearFactor(to_bt(a_collider.m_linearFactor));
		rigidbody->setAngularFactor(to_bt(a_collider.m_angularFactor));
		rigidbody->setRestitution(material.m_restitution);
		rigidbody->setFriction(material.m_friction);
		rigidbody->setRollingFriction(material.m_rollingFriction);
		rigidbody->setSpinningFriction(material.m_spinningFriction);
		rigidbody->setContactStiffnessAndDamping(
			material.m_contactStiffness, material.m_contactDamping);

		return a_entity.emplace<rigidbody_component>(std::move(rigidbody));
	}*/
}
