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
		return;
		auto const dt = m_simulationTimeWorldComponent->elapsed_time.get_value();
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
					m_debugMeshWorldComponent->add_line(contactPosition + kh, contactPosition + lateralFrictionForce + kh, aoegl::k_magenta);
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

	bool are_intersecting(aabb const& a_lhs, aabb const& a_rhs)
	{
		if (a_lhs.max.x < a_rhs.min.x || a_lhs.max.y < a_rhs.min.y || a_lhs.max.z < a_rhs.min.z)
		{
			return false;
		}

		if (a_rhs.max.x < a_lhs.min.x || a_rhs.max.y < a_lhs.min.y || a_rhs.max.z < a_lhs.min.z)
		{
			return false;
		}

		return true;
	}

	struct rk4_state
	{
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 linearVelocity;
		glm::vec3 angularVelocityLocal;
	};

	using broadphase_result = std::tuple<aoest::position, aoest::rotation, static_body>;

	static inline glm::vec3 closest_triangle_point(triangle const& a_triangle, glm::vec3 const& a_point)
	{
		auto const v01 = a_triangle.p1 - a_triangle.p0;
		auto const v02 = a_triangle.p2 - a_triangle.p0;
		auto const v0p = a_point - a_triangle.p0;
		
		auto const d1 = glm::dot(v01, v0p);
		auto const d2 = glm::dot(v02, v0p);
		if (d1 <= 0.0f && d2 <= 0.0f)
		{
			return a_triangle.p0;
		}

		auto const v13 = a_point - a_triangle.p1;
		auto const d3 = glm::dot(v01, v13);
		auto const d4 = glm::dot(v02, v13);
		if (d3 >= 0.0f && d4 <= d3)
		{
			return a_triangle.p1;
		}

		auto const v2 = d1 * d4 - d2 * d3;
		if (v2 <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
		{
			return a_triangle.p0 + v01 * d1 / (d1 - d3);
		}

		auto const v2p = a_point - a_triangle.p2;
		auto const d5 = glm::dot(v01, v2p);
		auto const d6 = glm::dot(v02, v2p);
		if (d6 >= 0.0f && d5 <= d6)
		{
			return a_triangle.p2;
		}

		auto const v1 = d2 * d5 - d1 * d6;
		if (v1 <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
		{
			return a_triangle.p0 + v02 * d2 / (d2 - d6);
		}

		auto const v0 = d3 * d6 - d4 * d5;
		if (v0 <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
		{
			return a_triangle.p1 + (a_triangle.p2 - a_triangle.p1) * (d4 - d3) / ((d4 - d3) + (d5 - d6));
		}

		auto const d01Sq = glm::dot(v01, v01);
		auto const d02Sq = glm::dot(v02, v02);
		auto const d0102 = glm::dot(v01, v02);
		
		auto const d = d01Sq * d02Sq - d0102 * d0102;
		auto const v = (d02Sq * glm::dot(v0p, v01) - d0102 * glm::dot(v0p, v02)) / d;
		auto const w = (d01Sq * glm::dot(v0p, v02) - d0102 * glm::dot(v0p, v01)) / d;
		return a_triangle.p0 + v01 * v + v02 * w;
	}

	static inline glm::vec3 closest_unit_ellipsoid_point(glm::vec3 const& a_radiuses, glm::vec3 const& a_point)
	{
		auto const r2 = a_radiuses * a_radiuses;
		auto const computeError = [&a_point, &a_radiuses, &r2](const float lambda) {
			auto const p2 = a_point * a_point;
			auto const dSqrt = r2 + lambda;
			auto const t = p2 * r2 / (dSqrt * dSqrt);
			return t.x + t.y + t.z - 1.0f;
			};

		auto lambdaMin = -std::min({ r2.x, r2.y, r2.z });
		auto lambdaMax = 0.0f;
		while (lambdaMax - lambdaMin > 1e-4f)
		{
			auto const lambda = (lambdaMin + lambdaMax) * 0.5f;
			auto const error = computeError(lambda);
			if (error < 0.0f)
			{
				lambdaMax = lambda;
			}
			else
			{
				lambdaMin = lambda;
			}
		}

		auto const lambda = (lambdaMin + lambdaMax) * 0.5f;
		return a_point * r2 / (r2 + lambda);
	}

	std::tuple<float, glm::vec3, glm::vec3> intersect_unit_ellipsoid_with_triangle(
		glm::vec3 const& a_radiuses,
		triangle const& a_triangle)
	{
		auto const normal = glm::normalize(glm::cross(a_triangle.p1 - a_triangle.p0, a_triangle.p2 - a_triangle.p0));
		if (glm::dot(normal, -a_triangle.p0) < 0.0f)
		{
			return {};
		}

		auto const normalEllipsoidDir = normal * a_radiuses * a_radiuses;
		auto const normalEllipsoidPoint = -normalEllipsoidDir / std::sqrt(glm::dot(normalEllipsoidDir, normal));
		auto const normalDistance = glm::dot(normalEllipsoidPoint - a_triangle.p0, normal);
		auto const normalTrianglePoint = normalEllipsoidPoint - normalDistance * normal;

		auto const trianglePoint = closest_triangle_point(a_triangle, normalTrianglePoint);
		auto const ellipsoidPoint = closest_unit_ellipsoid_point(a_radiuses, trianglePoint);
		auto const distance = glm::length(trianglePoint - ellipsoidPoint);
		auto const t = trianglePoint / a_radiuses;
		return { glm::dot(t, t) < 1.0f ? -distance : distance, ellipsoidPoint, trianglePoint };
	}

	std::tuple<float, glm::vec3, glm::vec3> intersect_ellipsoid_with_triangle(
		glm::mat4 const& a_ellipsoidTransform,
		glm::mat4 const& a_ellipsoidTransformInv,
		glm::vec3 const& a_radiuses,
		triangle const& a_triangle)
	{
		auto [distance, unitEllipsoidPoint, unitTrianglePoint] = intersect_unit_ellipsoid_with_triangle(
			a_radiuses,
			triangle{
				aoest::apply(a_ellipsoidTransformInv, a_triangle.p0),
				aoest::apply(a_ellipsoidTransformInv, a_triangle.p1),
				aoest::apply(a_ellipsoidTransformInv, a_triangle.p2)
			});

		auto const ellipsoidPoint = aoest::apply(a_ellipsoidTransform, unitEllipsoidPoint);
		auto const trianglePoint = aoest::apply(a_ellipsoidTransform, unitTrianglePoint);
		return { distance, ellipsoidPoint, trianglePoint };
	}

	glm::quat differentiate_quaternion(glm::quat const& a_rotation, glm::vec3 const& a_angularVelocity)
	{
		auto const angularVelocity = glm::quat{ 0.0f, a_angularVelocity.x, a_angularVelocity.y, a_angularVelocity.z };
		return 0.5f * a_rotation * angularVelocity;
	}

	rk4_state rk4_derivate(dynamic_body const& a_dynamicBody, rk4_state const& a_state, std::vector<broadphase_result> const& a_broadphaseResults)
	{
		auto const rotationMatrix = glm::mat3_cast(a_state.rotation);
		auto const rotationMatrixInv = glm::transpose(rotationMatrix);
		auto const inertia = rotationMatrix * a_dynamicBody.inertia * glm::transpose(rotationMatrix);
		auto const inertiaInv = glm::inverse(inertia);

		struct contact
		{
			physx_material material;
			float distance = 0.0f;
			glm::vec3 ellipsoidPoint;
			glm::vec3 trianglePoint;
		};

		std::vector<contact> partContacts;
		partContacts.reserve(a_dynamicBody.parts.size());
		for (auto const& part : a_dynamicBody.parts)
		{
			auto const ellipsoidTransform = aoest::combine(
				a_state.position + rotationMatrix * part.position,
				a_state.rotation * part.rotation);
			auto const ellipsoidTransformInv = glm::inverse(ellipsoidTransform);

			contact closestContact;
			for (auto const& [staticBodyPosition, staticBodyRotation, staticBody] : a_broadphaseResults)
			{
				auto const staticBodyTransform = aoest::combine(staticBodyPosition, staticBodyRotation);

				for (auto const& staticBodyPart : staticBody.parts)
				{
					for (auto const& staticTriangle : staticBodyPart.triangles)
					{
						auto [distance, ellipsoidPoint, trianglePoint] = intersect_ellipsoid_with_triangle(
							ellipsoidTransform,
							ellipsoidTransformInv,
							part.radiuses,
							triangle{
								aoest::apply(staticBodyTransform, staticTriangle.p0),
								aoest::apply(staticBodyTransform, staticTriangle.p1),
								aoest::apply(staticBodyTransform, staticTriangle.p2) });
						if (distance < closestContact.distance)
						{
							closestContact = contact{ staticBodyPart.material, distance, ellipsoidPoint, trianglePoint };
						}
					}
				}
			}

			if (closestContact.distance < 0.0f)
			{
				partContacts.emplace_back(closestContact);
			}
		}

		auto force = a_dynamicBody.force;
		auto torque = a_dynamicBody.torque;
		for (auto const& contact : partContacts)
		{
			auto const lever = contact.ellipsoidPoint - a_state.position;
			auto const hitVelocity = a_state.linearVelocity + glm::cross(a_state.angularVelocityLocal, lever);
			auto const hitNormal = glm::normalize(contact.trianglePoint - contact.ellipsoidPoint);
			auto const hitVelocityNormal = glm::dot(hitVelocity, hitNormal) * hitNormal;
			auto const hitVelocityTangent = hitVelocity - hitVelocityNormal;

			auto const springForce = contact.material.ellasticity * (-contact.distance) * hitNormal;

			auto const hitSpeedNormal = glm::length(hitVelocityNormal);
			auto const zeta = glm::mix(contact.material.zetaHigh, contact.material.zetaLow, glm::smoothstep(0.01f, 0.2f, hitSpeedNormal));
			auto const dampingCoefficient = 2.0f * std::sqrt(contact.material.ellasticity * a_dynamicBody.mass / partContacts.size()) * zeta;
			auto const dampenerForce = -dampingCoefficient * hitVelocityNormal;

			auto frictionForce = glm::vec3{ 0.0f };
			if (glm::dot(hitVelocityNormal, hitVelocityNormal) > glm::epsilon<float>() * glm::epsilon<float>())
			{
				auto const frictionDir = -glm::normalize(hitVelocityNormal);
				auto const maxFriction = contact.material.friction * glm::length(springForce);
				frictionForce = frictionDir * std::min(maxFriction, glm::length(hitVelocityNormal) * a_dynamicBody.mass / partContacts.size());
			}

			force += springForce + dampenerForce + frictionForce;
			torque += glm::cross(lever, springForce + dampenerForce + frictionForce);
		}

		rk4_state derivativeState;
		derivativeState.position = a_state.linearVelocity;
		derivativeState.linearVelocity = force / a_dynamicBody.mass;
		derivativeState.rotation = differentiate_quaternion(a_state.rotation, rotationMatrixInv * a_state.angularVelocityLocal);
		derivativeState.angularVelocityLocal = inertiaInv * (torque - glm::cross(a_state.angularVelocityLocal, inertia * a_state.angularVelocityLocal));
		return derivativeState;
	}

	rk4_state rk4_step(rk4_state const& a_initialState, rk4_state const& a_prevDerivativeState, float a_stepDuration)
	{
		rk4_state state = a_initialState;
		state.position += a_prevDerivativeState.position * a_stepDuration;
		state.rotation = glm::normalize(state.rotation + a_prevDerivativeState.rotation * a_stepDuration);
		state.linearVelocity += a_prevDerivativeState.linearVelocity * a_stepDuration;
		state.angularVelocityLocal += a_prevDerivativeState.angularVelocityLocal * a_stepDuration;

		return state;
	}

	physx_debug_system::physx_debug_system(aoeng::world_data_provider& a_wdp)
		: m_physicsDebugContext{ a_wdp }
		, m_debugMeshContext{ a_wdp }
		, m_dynamicBodyEntities{ a_wdp }
		, m_staticBodyEntities{ a_wdp }
	{

	}

	void draw_ellipsoid(aoegl::debug_mesh_world_component& a_debugMeshContext, glm::mat4 const& a_transform, glm::vec3 const& a_radiuses, aoegl::rgba const& a_color)
	{
		constexpr auto kHorizontalSlices = 7;
		constexpr auto kHorizontalSubdivisions = 8;
		constexpr auto kVerticalSlices = 8;
		constexpr auto kVerticalSubdivisions = 8;

		for (int h = 0; h < kHorizontalSlices; ++h)
		{
			auto const hSliceAngle0 = (static_cast<float>(h) / (kHorizontalSlices + 1) - 0.5f) * std::numbers::pi_v<float>;
			auto const hSliceAngle1 = (static_cast<float>(h + 1) / (kHorizontalSlices + 1) - 0.5f) * std::numbers::pi_v<float>;
			for (int hs = 0; hs < kHorizontalSubdivisions; ++hs)
			{
				auto const hSubR0 = static_cast<float>(hs) / kHorizontalSubdivisions;
				auto const hSubR1 = static_cast<float>(hs + 1) / kHorizontalSubdivisions;
				auto const hSubAngle0 = hSliceAngle0 + hSubR0 * (hSliceAngle1 - hSliceAngle0);
				auto const hSubAngle1 = hSliceAngle0 + hSubR1 * (hSliceAngle1 - hSliceAngle0);

				auto const y0 = a_radiuses.y * std::sin(hSubAngle0);
				auto const r0 = std::cos(hSubAngle0);
				auto const y1 = a_radiuses.y * std::sin(hSubAngle1);
				auto const r1 = std::cos(hSubAngle1);

				for (int v = 0; v < 2 * kVerticalSlices; ++v)
				{
					auto const vSliceAngle = (static_cast<float>(v) / kVerticalSlices) * std::numbers::pi_v<float>;
					auto const vSliceCos = std::cos(vSliceAngle);
					auto const vSliceSin = std::sin(vSliceAngle);
					auto const localPos0 = glm::vec3{ r0 * a_radiuses.x * vSliceSin, y0, r0 * a_radiuses.z * vSliceCos };
					auto const localPos1 = glm::vec3{ r1 * a_radiuses.x * vSliceSin, y1, r1 * a_radiuses.z * vSliceCos };
					a_debugMeshContext.add_line(aoest::apply(a_transform, localPos0), aoest::apply(a_transform, localPos1), a_color);
				}
			}

			for (int v = 0; v < 2 * kVerticalSlices; ++v)
			{
				auto const vSliceAngle0 = (static_cast<float>(v) / kVerticalSlices) * std::numbers::pi_v<float>;
				auto const vSliceAngle1 = (static_cast<float>(v + 1) / kVerticalSlices) * std::numbers::pi_v<float>;

				for (int vs = 0; vs < kVerticalSubdivisions; ++vs)
				{
					auto const r = std::cos(hSliceAngle1);
					auto const y = a_radiuses.y * std::sin(hSliceAngle1);

					auto const vSubR0 = static_cast<float>(vs) / kVerticalSubdivisions;
					auto const vSubR1 = static_cast<float>(vs + 1) / kVerticalSubdivisions;
					auto const vSubAngle0 = vSliceAngle0 + vSubR0 * (vSliceAngle1 - vSliceAngle0);
					auto const vSubAngle1 = vSliceAngle0 + vSubR1 * (vSliceAngle1 - vSliceAngle0);

					auto const vSubCos0 = std::cos(vSubAngle0);
					auto const vSubSin0 = std::sin(vSubAngle0);
					auto const vSubCos1 = std::cos(vSubAngle1);
					auto const vSubSin1 = std::sin(vSubAngle1);
					auto const localPos0 = glm::vec3{ r * a_radiuses.x * vSubSin0, y, r * a_radiuses.z * vSubCos0 };
					auto const localPos1 = glm::vec3{ r * a_radiuses.x * vSubSin1, y, r * a_radiuses.z * vSubCos1 };
					a_debugMeshContext.add_line(aoest::apply(a_transform, localPos0), aoest::apply(a_transform, localPos1), a_color);
				}
			}
		}

		auto const hSliceAngle0 = (static_cast<float>(kHorizontalSlices) / (kHorizontalSlices + 1) - 0.5f) * std::numbers::pi_v<float>;
		auto const hSliceAngle1 = (static_cast<float>(kHorizontalSlices + 1) / (kHorizontalSlices + 1) - 0.5f) * std::numbers::pi_v<float>;
		for (int hs = 0; hs < kHorizontalSubdivisions; ++hs)
		{
			auto const hSubR0 = static_cast<float>(hs) / kHorizontalSubdivisions;
			auto const hSubR1 = static_cast<float>(hs + 1) / kHorizontalSubdivisions;
			auto const hSubAngle0 = hSliceAngle0 + hSubR0 * (hSliceAngle1 - hSliceAngle0);
			auto const hSubAngle1 = hSliceAngle0 + hSubR1 * (hSliceAngle1 - hSliceAngle0);

			auto const y0 = a_radiuses.y * std::sin(hSubAngle0);
			auto const r0 = std::cos(hSubAngle0);
			auto const y1 = a_radiuses.y * std::sin(hSubAngle1);
			auto const r1 = std::cos(hSubAngle1);

			for (int v = 0; v < 2 * kVerticalSlices; ++v)
			{
				auto const vSliceAngle = (static_cast<float>(v) / kVerticalSlices) * std::numbers::pi_v<float>;
				auto const vSliceCos = std::cos(vSliceAngle);
				auto const vSliceSin = std::sin(vSliceAngle);
				auto const localPos0 = glm::vec3{ r0 * a_radiuses.x * vSliceSin, y0, r0 * a_radiuses.z * vSliceCos };
				auto const localPos1 = glm::vec3{ r1 * a_radiuses.x * vSliceSin, y1, r1 * a_radiuses.z * vSliceCos };
				a_debugMeshContext.add_line(aoest::apply(a_transform, localPos0), aoest::apply(a_transform, localPos1), a_color);
			}
		}
	}
	
	void draw_triangle(aoegl::debug_mesh_world_component& a_debugMeshContext, glm::mat4 const& a_transform, triangle const a_triangle, aoegl::rgba const& a_color)
	{
		auto const p0 = aoest::apply(a_transform, a_triangle.p0);
		auto const p1 = aoest::apply(a_transform, a_triangle.p1);
		auto const p2 = aoest::apply(a_transform, a_triangle.p2);

		a_debugMeshContext.add_line(p0, p1, a_color);
		a_debugMeshContext.add_line(p1, p2, a_color);

		auto const s01Length = glm::length(p1 - p0);
		auto const s12Length = glm::length(p2 - p1);

		auto const subdivisionCount = static_cast<std::int32_t>(std::ceil(s01Length / 5.0f));
		for (auto subdivisionIndex = 0; subdivisionIndex < subdivisionCount; ++subdivisionIndex)
		{
			auto const subdivisionRatio = static_cast<float>(subdivisionIndex) / subdivisionCount;

			a_debugMeshContext.add_line(p2 + (p1 - p2) * subdivisionRatio, p0 + (p1 - p0) * subdivisionRatio, a_color);
		}
	}

	void physx_debug_system::update() const
	{
		auto physicsDebugContext = m_physicsDebugContext.get();
		auto dynamicBodyEntitiesView = m_dynamicBodyEntities.get();
		auto staticBodyEntitiesView = m_staticBodyEntities.get();
		
		for (auto const dynamicBodyEntity : dynamicBodyEntitiesView)
		{
			auto [position, rotation, linearVelocity, angularVelocityLocal, dynamicBody] = dynamicBodyEntitiesView.get(dynamicBodyEntity);
			auto const transform = aoest::combine4x3(position, rotation);

			auto const rotationMatrix = glm::mat3_cast(rotation);
			if (physicsDebugContext.is_dynamic_shape_debug_enabled)
			{
				for (auto const& part : dynamicBody.parts)
				{
					auto const ellipsoidTransform = aoest::combine(position + rotationMatrix * part.position, rotation * part.rotation);
					draw_ellipsoid(*m_debugMeshContext, ellipsoidTransform, part.radiuses, aoegl::k_red);
				}
			}
		}

		for (auto const staticBodyEntity : staticBodyEntitiesView)
		{
			auto [position, rotation, staticBody] = staticBodyEntitiesView.get(staticBodyEntity);

			auto const staticBodyTransform = aoest::combine(position, rotation);

			for (auto const& part : staticBody.parts)
			{
				for (auto const& triangle : part.triangles)
				{
					draw_triangle(*m_debugMeshContext, staticBodyTransform, triangle, aoegl::k_white);
				}
			}
		}
	}

	physx_system::physx_system(aoeng::world_data_provider& a_wdp)
		: m_simulationTimeContext{ a_wdp }
		, m_physicsContext{ a_wdp }
		, m_inputs{ a_wdp }
		, m_dynamicBodyEntities{ a_wdp }
		, m_staticBodyEntities{ a_wdp }
	{
	}

#pragma optimize("", off)
	void physx_system::update() const
	{
		auto dynamicBodyEntitiesView = m_dynamicBodyEntities.get();
		auto staticBodyEntitiesView = m_staticBodyEntities.get();

		auto& simulationTimeContext = m_simulationTimeContext.get();
		auto& physicsContext = m_physicsContext.get();

		//
		if (m_inputs->keyboard.keys[aoein::keyboard::key::P].is_pressed())
		{
			for (auto const dynamicBodyEntity : dynamicBodyEntitiesView)
			{
				// broadphase
				auto [position, rotation, linearVelocity, angularVelocityLocal, dynamicBody] = dynamicBodyEntitiesView.get(dynamicBodyEntity);

				position.y = 10.0f;
				linearVelocity.y = 0.0f;
			}
		}

		if (physicsContext.m_lastUpdateTime + physx_context::clock::duration{ physicsContext.m_updateDuration } < simulationTimeContext.tick_start_time)
		{
			physicsContext.m_lastUpdateTime = physicsContext.m_lastUpdateTime + physx_context::clock::duration{ physicsContext.m_updateDuration };

			auto const simulationTimeStep = physicsContext.m_updateDuration.get_value() / physicsContext.m_updateStepCount;

			std::vector<std::tuple<aoest::position, aoest::rotation, static_body>> broadphaseResults;

			for (auto const dynamicBodyEntity : dynamicBodyEntitiesView)
			{
				broadphaseResults.clear();

				// broadphase
				auto [position, rotation, linearVelocity, angularVelocityLocal, dynamicBody] = dynamicBodyEntitiesView.get(dynamicBodyEntity);

				auto const transform = aoest::combine4x3(position, rotation);
				auto boundsMin = glm::vec3{ std::numeric_limits<float>::max() };
				auto boundsMax = glm::vec3{ std::numeric_limits<float>::min() };
				for (auto const& part : dynamicBody.parts)
				{
					auto const maxRadius = glm::vec3{ std::max({ part.radiuses.x, part.radiuses.y, part.radiuses.z }) };
					auto const partPosition = transform * glm::vec4{ part.position, 1.0f };
					boundsMin = glm::min(boundsMin, partPosition - maxRadius);
					boundsMax = glm::max(boundsMax, partPosition + maxRadius);
				}
				dynamicBody.bounds = aabb{ boundsMin - glm::vec3{1.0f}, boundsMax + glm::vec3{1.0f} };

				for (auto const staticBodyEntity : staticBodyEntitiesView)
				{
					auto [staticBodyPosition, staticBodyRotation, staticBody] = staticBodyEntitiesView.get(staticBodyEntity);

					if (are_intersecting(dynamicBody.bounds, staticBody.bounds))
					{
						broadphaseResults.emplace_back(staticBodyPosition, staticBodyRotation, staticBody);
					}
				}

				// narrowphase
				auto const updateStepDuration = physicsContext.m_updateDuration.get_value() / physicsContext.m_updateStepCount;
				for (int32_t updateStep = 0; updateStep < physicsContext.m_updateStepCount; ++updateStep)
				{
					auto const initialState = rk4_state{ position, rotation, linearVelocity, angularVelocityLocal };
					auto const k1 = rk4_derivate(dynamicBody, initialState, broadphaseResults);
					auto const k2 = rk4_derivate(dynamicBody, rk4_step(initialState, k1, updateStepDuration * 0.5f), broadphaseResults);
					auto const k3 = rk4_derivate(dynamicBody, rk4_step(initialState, k2, updateStepDuration * 0.5f), broadphaseResults);
					auto const k4 = rk4_derivate(dynamicBody, rk4_step(initialState, k3, updateStepDuration), broadphaseResults);

					position += (simulationTimeStep / 6.0f) * (k1.position + 2.0f * k2.position + 2.0f * k3.position + k4.position);
					rotation += glm::normalize(rotation + (simulationTimeStep / 6.0f) * (k1.rotation + 2.0f * k2.rotation + 2.0f * k3.rotation + k4.rotation));
					linearVelocity += (simulationTimeStep / 6.0f) * (k1.linearVelocity + 2.0f * k2.linearVelocity + 2.0f * k3.linearVelocity + k4.linearVelocity);
					angularVelocityLocal += (simulationTimeStep / 6.0f) * (k1.angularVelocityLocal + 2.0f * k2.angularVelocityLocal + 2.0f * k3.angularVelocityLocal + k4.angularVelocityLocal);
				}

				dynamicBody.force = glm::vec3{ 0.0f, -10.0f, 0.0f } / dynamicBody.mass;
				dynamicBody.torque = glm::vec3{ 0.0f };
			}
		}
	}
}
