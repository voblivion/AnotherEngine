#include <vob/aoe/physics/physics_system.h>

#include <vob/aoe/physics/maths.h>

#include "imgui.h"
#include "optick.h"

namespace vob::aoeph
{
	inline aabb compute_car_bounds(glm::mat4x3 const& a_carTransform, car_collider const& a_carCollider)
	{
		auto bounds = aabb{ glm::vec3{ std::numeric_limits<float>::max() }, glm::vec3{ -std::numeric_limits<float>::max() } };

		for (auto const& chassisPart : a_carCollider.chassisParts)
		{
			auto const partMaxRadius = glm::vec3{ std::max({ chassisPart.radiuses.x, chassisPart.radiuses.y, chassisPart.radiuses.z }) };
			auto const partPosition = a_carTransform * glm::vec4{ chassisPart.position, 1.0f };
			bounds.min = glm::min(bounds.min, partPosition - partMaxRadius);
			bounds.max = glm::max(bounds.max, partPosition + partMaxRadius);
		}

		for (auto const& wheel : a_carCollider.wheels)
		{
			auto const wheelMaxRadius = glm::vec3{ std::max({ wheel.radiuses.x, wheel.radiuses.y, wheel.radiuses.z }) };
			auto const wheelHighPosition = a_carTransform * glm::vec4{ wheel.attachPosition, 1.0f };
			bounds.min = glm::min(bounds.min, wheelHighPosition - wheelMaxRadius);
			bounds.max = glm::max(bounds.max, wheelHighPosition + wheelMaxRadius);
			auto const wheelLowPosition = wheelHighPosition - wheel.rotation * glm::vec3{ 0.0f, -wheel.suspensionMaxLength, 0.0f };
			bounds.min = glm::min(bounds.min, wheelLowPosition - wheelMaxRadius);
			bounds.max = glm::max(bounds.max, wheelLowPosition + wheelMaxRadius);
		}

		return bounds;
	}

	/* void physics_system::update() const
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
	}*/

	struct rk4_state
	{
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 linearVelocity;
		glm::vec3 angularVelocityLocal;
	};

	using broadphase_result = std::tuple<triangle, material>;

	rk4_state rk4_derivate(dynamic_body const& a_dynamicBody, rk4_state const& a_state, std::vector<broadphase_result> const& a_broadphaseResults)
	{
		OPTICK_EVENT("RK4");
		auto const rotationMatrix = glm::mat3_cast(a_state.rotation);
		auto const rotationMatrixInv = glm::transpose(rotationMatrix);
		auto const inertia = rotationMatrix * a_dynamicBody.inertia * glm::transpose(rotationMatrix);
		auto const inertiaInv = glm::inverse(inertia);
		auto const barycenterPosition = a_state.position + rotationMatrix * a_dynamicBody.barycenter;

		struct contact
		{
			material material;
			float distance = 0.0f;
			glm::vec3 ellipsoidPoint;
			glm::vec3 trianglePoint;
		};

		std::vector<contact> partContacts;
		partContacts.reserve(a_dynamicBody.parts.size());
		for (auto const& part : a_dynamicBody.parts)
		{
			OPTICK_EVENT("Part");
			auto const partPosition = a_state.position + rotationMatrix * part.position;
			auto const ellipsoidTransform = aoest::combine(partPosition, a_state.rotation * part.rotation);
			auto const ellipsoidTransformInv = glm::inverse(ellipsoidTransform);

			auto const dynamicPartApproximateAabb = compute_ellipsoid_approximate_aabb(partPosition, part.radiuses);

			contact closestContact;
			{
				OPTICK_EVENT("Closest");
				for (auto const& [staticTriangle, material] : a_broadphaseResults)
				{
					auto intersection = intersect_ellipsoid_with_triangle(
						ellipsoidTransform,
						ellipsoidTransformInv,
						part.radiuses,
						staticTriangle);
					if (intersection.signedDistance < closestContact.distance)
					{
						closestContact = contact{ material, intersection.signedDistance, intersection.firstPoint, intersection.secondPoint };
					}
				}
			}

			if (closestContact.distance < 0.0f)
			{
				partContacts.emplace_back(closestContact);

				// TODO: debug only
				part.debug_contact = dynamic_body::part::contact{ closestContact.ellipsoidPoint, closestContact.trianglePoint };
			}
		}

		OPTICK_EVENT("Contact");
		auto force = a_dynamicBody.force;
		auto torque = a_dynamicBody.torque;
		for (auto const& contact : partContacts)
		{
			auto const lever = contact.ellipsoidPoint - barycenterPosition;
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
			if (glm::dot(hitVelocityTangent, hitVelocityTangent) > glm::epsilon<float>() * glm::epsilon<float>())
			{
				auto const frictionDir = -glm::normalize(hitVelocityTangent);
				auto const maxFriction = contact.material.friction * glm::length(springForce);
				frictionForce = frictionDir * std::min(maxFriction, glm::length(hitVelocityNormal) * a_dynamicBody.mass / partContacts.size());
			}

			force += springForce + dampenerForce + frictionForce;
			torque += glm::cross(lever, springForce + dampenerForce + frictionForce);
		}

		rk4_state derivativeState;
		derivativeState.position = a_state.linearVelocity;
		derivativeState.linearVelocity = force / a_dynamicBody.mass;
		// TODO : are we sure it's inv?
		derivativeState.rotation = differentiate_quaternion(a_state.rotation, rotationMatrixInv * a_state.angularVelocityLocal);
		derivativeState.angularVelocityLocal = inertiaInv * (torque - glm::cross(a_state.angularVelocityLocal, inertia * a_state.angularVelocityLocal));
		return derivativeState;
	}

#pragma optimize("", off)
	rk4_state rk4_derivate(
		car_collider& a_carCollider,
		rk4_state const& a_state,
		std::vector<broadphase_result> const& a_broadphaseResults,
		std::vector<std::optional<std::pair<glm::vec3, glm::vec3>>>& a_debugChassisContacts,
		std::vector<std::optional<std::pair<glm::vec3, glm::vec3>>>& a_debugRimContacts,
		std::vector<std::optional<std::pair<glm::vec3, glm::vec3>>>& a_debugTireContacts)
	{
		auto const rotationMatrix = glm::mat3_cast(a_state.rotation);
		auto const rotationMatrixInv = glm::transpose(rotationMatrix);
		auto const inertia = rotationMatrix * a_carCollider.inertia * glm::transpose(rotationMatrix);
		auto const inertiaInv = glm::inverse(inertia);
		auto const barycenterPosition = a_state.position + rotationMatrix * a_carCollider.barycenter;

		struct chassis_contact
		{
			material carMaterial;
			material staticMaterial;
			float distance = 0.0f;
			glm::vec3 carPoint;
			glm::vec3 staticPoint;
		};

		std::vector<chassis_contact> chassisContacts;
		chassisContacts.reserve(a_carCollider.chassisParts.size());
		int32_t c = 0;
		for (auto const& chassisPart : a_carCollider.chassisParts)
		{
			auto const partPosition = a_state.position + rotationMatrix * chassisPart.position;
			auto const partTransform = aoest::combine(partPosition, a_state.rotation * chassisPart.rotation);
			auto const partTransformInv = glm::inverse(partTransform);

			chassis_contact closestContact;
			for (auto const& [staticTriangle, staticMaterial] : a_broadphaseResults)
			{
				auto const intersectionResult = intersect_ellipsoid_with_triangle(
					partTransform, partTransformInv, chassisPart.radiuses, staticTriangle);

				if (intersectionResult.signedDistance < closestContact.distance)
				{
					closestContact = chassis_contact{
						chassisPart.material,
						staticMaterial,
						intersectionResult.signedDistance,
						intersectionResult.firstPoint,
						intersectionResult.secondPoint
					};
				}
			}

			if (closestContact.distance < 0.0f)
			{
				chassisContacts.push_back(closestContact);

				// TMP debug:
				a_debugChassisContacts[c] = std::pair(
					closestContact.staticPoint, aoest::normalize_safe(closestContact.staticPoint - closestContact.carPoint));
			}

			++c;
		}

		struct wheel_contact
		{
			material staticMaterial;
			float suspensionDisplacementRatio = 1.0f;
			float distance = 0.0f;
			glm::vec3 wheelPoint;
			glm::vec3 staticPoint;
			glm::vec3 contactNormal;
		};

		std::vector<wheel_contact> wheelContacts;
		wheelContacts.reserve(a_carCollider.wheels.size());
		for (auto const& wheel : a_carCollider.wheels)
		{
			auto const wheelAttachmentPosition = a_state.position + rotationMatrix * wheel.attachPosition;
			auto const wheelAttachmentRotation = a_state.rotation * wheel.rotation;
			auto const wheelAttachmentTransform = aoest::combine(wheelAttachmentPosition, wheelAttachmentRotation);
			auto const wheelAttachmentTransformInv = glm::inverse(wheelAttachmentTransform);

			auto const wheelDown = wheelAttachmentRotation * glm::vec3{ 0.0f, -1.0f, 0.0f };

			auto const suspensionDisplacement = wheel.suspensionMaxLength * wheelDown;

			wheel_contact wheelContact;
			for (auto const& [staticTriangle, staticMaterial] : a_broadphaseResults)
			{
				auto const intersectionResult = intersect_ellipsoid_with_triangle(
					wheelAttachmentTransform, wheelAttachmentTransformInv, wheel.radiuses, staticTriangle);
				if ((wheelContact.suspensionDisplacementRatio > 0.0f && intersectionResult.signedDistance < 0.0f)
					|| (wheelContact.suspensionDisplacementRatio == 0.0f && intersectionResult.signedDistance < wheelContact.distance))
				{
					wheelContact = wheel_contact{
						staticMaterial,
						0.0f,
						intersectionResult.signedDistance,
						intersectionResult.firstPoint,
						intersectionResult.secondPoint,
						glm::normalize(intersectionResult.secondPoint - intersectionResult.firstPoint)
					};

					continue;
				}

				auto const castResult = cast_ellipsoid_with_triangle(
					wheelAttachmentTransform, wheelAttachmentTransformInv, wheel.radiuses, suspensionDisplacement, staticTriangle);

				if (castResult.displacementRatio >= 0.0f && castResult.displacementRatio < wheelContact.suspensionDisplacementRatio)
				{
					wheelContact = wheel_contact{
						staticMaterial,
						castResult.displacementRatio,
						0.0f,
						castResult.point,
						castResult.point,
						castResult.normal
					};
				}
			}

			wheelContacts.push_back(wheelContact);
		}

		struct contact_parameters
		{
			float ellasticity;
			float restitution;
			float friction;
		};

		auto const mass = a_carCollider.mass;
		glm::vec3 force = a_carCollider.force;
		auto torque = a_carCollider.torque;
		auto computeRestitution = [&a_state, &barycenterPosition, &mass, &force, &torque](
				glm::vec3 const& a_carPoint, glm::vec3 const& a_staticPoint, float const& a_distance, contact_parameters const& a_contactParameters)
			{
				auto const lever = a_carPoint - barycenterPosition;
				auto const hitVelocity = a_state.linearVelocity + glm::cross(a_state.angularVelocityLocal, lever);
				auto const hitNormal = glm::normalize(a_staticPoint - a_carPoint);
				auto const hitVelocityNormal = glm::dot(hitVelocity, hitNormal) * hitNormal;
				auto const hitVelocityTangent = hitVelocity - hitVelocityNormal;

				// 1. spring
				auto const springForce = a_contactParameters.ellasticity * (-a_distance) * hitNormal;
				
				// 2. dampener
				auto const hitSpeedNormal = glm::length(hitVelocityNormal);
				auto const logRestitutionSquared = square(std::log(a_contactParameters.restitution));
				auto const piSquared = square(std::numbers::pi_v<float>);
				auto const zetaLow = std::sqrt(logRestitutionSquared / (logRestitutionSquared + piSquared));
				auto const zetaHigh = zetaLow;
				auto const zeta = glm::mix(zetaHigh, zetaLow, glm::smoothstep(0.0f, 0.2f, glm::smoothstep(0.01f, 0.2f, hitSpeedNormal)));
				// TODO: consider dividing mass by number of contact points?
				auto const dampingCoefficient = 2.0f * std::sqrt(a_contactParameters.ellasticity * mass) * zeta;
				auto const dampenerForce = -dampingCoefficient * hitVelocityNormal;

				// 3. friction
				auto frictionForce = glm::vec3{ 0.0f };
				if (glm::dot(hitVelocityTangent, hitVelocityTangent) > square(glm::epsilon<float>()))
				{
					auto const frictionDir = -glm::normalize(hitVelocityTangent);
					auto const maxFriction = a_contactParameters.friction * glm::length(springForce);
					// TODO: consider dividing mass by number of contact points?
					frictionForce = frictionDir * std::min(maxFriction, glm::length(hitVelocityNormal) * mass);
				}

				// 4. total
				auto const totalContactForce = springForce + dampenerForce + frictionForce;
				force += totalContactForce;
				torque += glm::cross(lever, totalContactForce);
			};

		for (auto const& chassisContact : chassisContacts)
		{
			auto const contactParameters = contact_parameters{
				1.0f / (1.0f / chassisContact.carMaterial.ellasticity + 1.0f / chassisContact.staticMaterial.ellasticity),
				std::max(chassisContact.carMaterial.restitution, chassisContact.staticMaterial.restitution),
				std::sqrt(chassisContact.carMaterial.friction * chassisContact.staticMaterial.friction)
			};

			computeRestitution(chassisContact.carPoint, chassisContact.staticPoint, chassisContact.distance, contactParameters);
		}

		for (int32_t w = 0; w < a_carCollider.wheels.size(); ++w)
		{
			auto& wheel = a_carCollider.wheels[w];
			auto const& wheelContact = wheelContacts[w];
			auto const wheelAttachmentRotation = a_state.rotation * wheel.rotation;
			auto const wheelForward = wheelAttachmentRotation * glm::vec3{ 0.0f, 0.0f, -1.0f };
			auto const wheelRight = wheelAttachmentRotation * glm::vec3{ 1.0f, 0.0f, 0.0f };
			auto const contactAngleSin = glm::dot(wheelRight, wheelContact.contactNormal);
			auto const tireMaxAngleSin = std::cos(std::numbers::pi_v<float> / 2.0f - wheel.tireMaxAngle);

			// Note: tm goes 0.01 min when falling? when accelerating on flat, can go 0.005, never seen lower
			// is it a 0.005 min threshold ?
			// 3 wheels touch => can still tilt (but barycenter must be slightly offset...)

			if (wheelContact.suspensionDisplacementRatio == 0.0f)
			{
				if (!(contactAngleSin <= tireMaxAngleSin))
				{
					auto const contactParameters = contact_parameters{
						1.0f / (1.0f / wheel.rimMaterial.ellasticity + 1.0f / wheelContact.staticMaterial.ellasticity),
						std::max(wheel.rimMaterial.restitution, wheelContact.staticMaterial.restitution),
						std::sqrt(wheel.rimMaterial.friction * wheelContact.staticMaterial.friction)
					};

					computeRestitution(wheelContact.wheelPoint, wheelContact.staticPoint, wheelContact.distance, contactParameters);

					// TMP debug:
					a_debugRimContacts[w] = std::pair(wheelContact.staticPoint, wheelContact.contactNormal);
				}
				else if (wheelContact.distance < -glm::epsilon<float>())
				{
					auto const contactParameters = contact_parameters{
						wheelContact.staticMaterial.ellasticity,
						0.01f /* restitution */,
						0.0f /* friction*/ };

					computeRestitution(wheelContact.wheelPoint, wheelContact.staticPoint, wheelContact.distance, contactParameters);

					// TMP debug:
					a_debugTireContacts[w] = std::pair(wheelContact.staticPoint, wheelContact.contactNormal);
				}
			}

			if (wheelContact.suspensionDisplacementRatio < 1.0f)
			{
				wheel.isGrounded = contactAngleSin <= tireMaxAngleSin;
				wheel.groundPosition = wheelContact.staticPoint;
				wheel.suspensionLength = wheelContact.suspensionDisplacementRatio * wheel.suspensionMaxLength;

				auto const springOffset = (1.0f - wheelContact.suspensionDisplacementRatio) * wheel.suspensionMaxLength;
				auto const wheelUp = wheelAttachmentRotation * glm::vec3{ 0.0f, 1.0f, 0.0f };
				auto const springForce = wheel.suspensionEllasticity * springOffset * wheelUp;

				auto const wheelAttachmentPosition = a_state.position + a_state.rotation * wheel.attachPosition;

				auto const lever = wheelAttachmentPosition - barycenterPosition;
				auto const hitVelocity = a_state.linearVelocity + glm::cross(a_state.angularVelocityLocal, lever);
				auto const hitVelocityNormal = glm::dot(hitVelocity, wheelUp) * wheelUp;
				auto const dampenerForce = -wheel.suspensionDamper * hitVelocityNormal;

				auto const totalSuspensionForce = springForce + dampenerForce;
				force += totalSuspensionForce;
				torque += glm::cross(lever, totalSuspensionForce);

				a_debugTireContacts[w] = std::pair(wheelContact.staticPoint, wheelContact.contactNormal);
			}
			else
			{
				wheel.isGrounded = false;
				wheel.suspensionLength = wheel.suspensionMaxLength;
			}
		}

		rk4_state derivativeState;
		derivativeState.position = a_state.linearVelocity;
		derivativeState.linearVelocity = force / a_carCollider.mass;
		// TODO : are we sure it's inv?
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

	physics_debug_system::physics_debug_system(aoeng::world_data_provider& a_wdp)
		: m_physicsDebugContext{ a_wdp }
		, m_debugMeshContext{ a_wdp }
		, m_dynamicBodyEntities{ a_wdp }
		, m_staticColliderEntities{ a_wdp }
		, m_carColliderEntities{ a_wdp }
	{

	}

	void physics_debug_system::update() const
	{
		auto& physicsDebugContext = m_physicsDebugContext.get();
		auto dynamicBodyEntitiesView = m_dynamicBodyEntities.get();
		
		/*for (auto const dynamicBodyEntity : dynamicBodyEntitiesView)
		{
			auto [position, rotation, linearVelocity, angularVelocityLocal, dynamicBody] = dynamicBodyEntitiesView.get(dynamicBodyEntity);
			auto const transform = aoest::combine4x3(position, rotation);

			auto const rotationMatrix = glm::mat3_cast(rotation);
			for (auto const& part : dynamicBody.parts)
			{
				if (physicsDebugContext.is_dynamic_shape_debug_enabled && part.debug_draw_enabled)
				{
					auto const ellipsoidTransform = aoest::combine(position + rotationMatrix * part.position, rotation * part.rotation);
					m_debugMeshContext->add_ellipsoid(ellipsoidTransform, part.radiuses, aoegl::k_white);

				}

				if (physicsDebugContext.is_dynamic_contact_debug_enabled)
				{
					if (part.debug_contact.has_value())
					{
						auto const ellipsoidPoint = part.debug_contact->ellipsoid_point;
						m_debugMeshContext->add_sphere(ellipsoidPoint, 0.05f, aoegl::k_red);
						m_debugMeshContext->add_sphere(part.debug_contact->static_point, 0.05f, aoegl::k_green);
						auto const contactDir = part.debug_contact->static_point - ellipsoidPoint;
						m_debugMeshContext->add_line(ellipsoidPoint, ellipsoidPoint + contactDir * 10000.0f, aoegl::k_red);
					}
				}
			}
			
			if (physicsDebugContext.is_dynamic_velocity_debug_enabled)
			{
				auto const barycenterPosition = position + rotationMatrix * dynamicBody.barycenter;
				m_debugMeshContext->add_line(barycenterPosition, barycenterPosition + linearVelocity, aoegl::k_azure);
				m_debugMeshContext->add_line(barycenterPosition, barycenterPosition + angularVelocityLocal, aoegl::k_blue);
			}
		}*/

		float suspensionLengths[4];
		float areGrounded[4];

		if (physicsDebugContext.is_dynamic_shape_debug_enabled)
		{
			for (auto [entity, position, rotation, linearVelocity, angularVelocityLocal, carCollider] : m_carColliderEntities.get().each())
			{
				auto const rotationMatrix = glm::mat3_cast(rotation);

				for (auto const& chassisPart : carCollider.chassisParts)
				{
					auto const ellipsoidTransform = aoest::combine(position + rotationMatrix * chassisPart.position, rotation * chassisPart.rotation);
					m_debugMeshContext->add_ellipsoid(ellipsoidTransform, chassisPart.radiuses, aoegl::k_white);

				}

				int32_t w = 0;
				for (auto const& wheel : carCollider.wheels)
				{
					areGrounded[w] = wheel.isGrounded ? 1.0f : 0.0f;
					suspensionLengths[w] = wheel.suspensionLength;
					++w;

					auto const wheelRotation = rotation * wheel.rotation;
					auto const wheelDown = wheelRotation * glm::vec3{ 0.0f, -1.0f, 0.0f };
					auto const wheelPosition = position + rotationMatrix * wheel.attachPosition + wheelRotation * (wheel.suspensionLength * wheelDown);

					auto const ellipsoidTransform = aoest::combine(wheelPosition, wheelRotation);
					m_debugMeshContext->add_ellipsoid(ellipsoidTransform, wheel.radiuses, aoegl::k_green);
				}
			}
		}

		if (physicsDebugContext.is_dynamic_contact_debug_enabled)
		{
			for (auto [entity, position, rotation, linearVelocity, angularVelocityLocal, carCollider] : m_carColliderEntities.get().each())
			{
				for (auto const& chassisPart : carCollider.chassisParts)
				{
					for (auto const& debugContact : chassisPart.debugContacts)
					{
						m_debugMeshContext->add_sphere(debugContact.groundPosition, 0.02f, aoegl::k_red);
						m_debugMeshContext->add_line(debugContact.groundPosition, debugContact.groundPosition + debugContact.groundNormal * 3.0f, aoegl::k_red);
					}
				}
				for (auto const& wheel : carCollider.wheels)
				{
					for (auto const& debugContact : wheel.debugRimContacts)
					{
						m_debugMeshContext->add_sphere(debugContact.groundPosition, 0.02f, aoegl::k_orange);
						m_debugMeshContext->add_line(debugContact.groundPosition, debugContact.groundPosition + debugContact.groundNormal * 3.0f, aoegl::k_orange);
					}
					for (auto const& debugContact : wheel.debugTireContacts)
					{
						m_debugMeshContext->add_sphere(debugContact.groundPosition, 0.02f, aoegl::k_green);
						m_debugMeshContext->add_line(debugContact.groundPosition, debugContact.groundPosition + debugContact.groundNormal * 3.0f, aoegl::k_green);
					}
				}
			}
		}

		if (physicsDebugContext.is_dynamic_contact_debug_enabled)
		{
			for (auto [entity, position, rotation, linearVelocity, angularVelocityLocal, carCollider] : m_carColliderEntities.get().each())
			{
				auto const carTransform = aoest::combine4x3(position, rotation);

				auto const carBounds = compute_car_bounds(carTransform, carCollider);
				auto const searchBounds = aabb{ carBounds.min - glm::vec3{3.0f}, carBounds.max + glm::vec3{3.0f} };

				m_debugMeshContext->add_aabb(searchBounds.min, searchBounds.max, aoegl::k_azure);
			}
		}

		if (physicsDebugContext.is_static_shape_debug_enabled)
		{
			for (auto [entity, position, rotation, staticCollider] : m_staticColliderEntities.get().each())
			{
				auto const staticBodyTransform = aoest::combine(position, rotation);

				for (auto const& part : staticCollider.parts)
				{
					int32_t t = 0;
					for (auto const& triangle : part.triangles)
					{
						m_debugMeshContext->add_triangle(
							aoest::apply(staticBodyTransform, triangle.p0),
							aoest::apply(staticBodyTransform, triangle.p1),
							aoest::apply(staticBodyTransform, triangle.p2),
							std::find(part.debugTriangleIndices.begin(), part.debugTriangleIndices.end(), t) != part.debugTriangleIndices.end()
								? aoegl::k_green : aoegl::k_gray);

						++t;
					}
				}
			}
		}

		ImGui::Begin("Physics Debug");
		ImGui::Checkbox("Draw Static Colliders", &physicsDebugContext.is_static_shape_debug_enabled);
		ImGui::Checkbox("Draw Car Colliders", &physicsDebugContext.is_dynamic_shape_debug_enabled);
		ImGui::Checkbox("Draw Contact Points", &physicsDebugContext.is_dynamic_contact_debug_enabled);
		ImGui::InputFloat4("Suspension Lengths", suspensionLengths);
		ImGui::InputFloat4("Are Grounded", areGrounded);
		ImGui::End();

	}

#pragma optimize("", off)
	inline aabb compute_triangle_bounds(triangle const& a_triangle)
	{
		auto bounds = aabb{ a_triangle.p0, a_triangle.p0 };

		bounds.min = glm::min(bounds.min, a_triangle.p1);
		bounds.max = glm::max(bounds.max, a_triangle.p1);
		bounds.min = glm::min(bounds.min, a_triangle.p2);
		bounds.max = glm::max(bounds.max, a_triangle.p2);

		return bounds;

	}

	physics_system::physics_system(aoeng::world_data_provider& a_wdp)
		: m_simulationTimeContext{ a_wdp }
		, m_physicsContext{ a_wdp }
		, m_inputs{ a_wdp }
		, m_dynamicBodyEntities{ a_wdp }
		, m_staticColliderEntities{ a_wdp }
		, m_carColliderEntities{ a_wdp }
	{
	}

	void physics_system::update() const
	{
		auto dynamicBodyEntitiesView = m_dynamicBodyEntities.get();
		auto staticBodyEntitiesView = m_staticColliderEntities.get();

		auto& simulationTimeContext = m_simulationTimeContext.get();
		auto& physicsContext = m_physicsContext.get();


		static glm::vec3 k_respawnPosition = glm::vec3{0.0f, 13.0f, 100.0f};
		static glm::vec3 k_respawnRotation = glm::vec3{0.0f};
		//
		if (m_inputs->gamepads[0].buttons[aoein::gamepad::button::Y].is_pressed()
			|| m_inputs->keyboard.keys[aoein::keyboard::key::P].is_pressed())
		{
			for (auto [carEntity, carPosition, carRotation, carLinearVelocity, carAngularVelocityLocal, carCollider] : m_carColliderEntities.get().each())
			{
				carPosition = k_respawnPosition - carCollider.barycenter;
				carRotation = glm::quat(k_respawnRotation);
				carLinearVelocity = glm::vec3{ 0.0f, 0.0f, 0.0f };
				carAngularVelocityLocal = glm::vec3{0.0f};
			}
		}

		if (m_inputs->keyboard.keys[aoein::keyboard::key::Z].was_pressed())
		{
			simulationTimeContext.play_for_duration = misph::measure_time(1.0) / 100;
		}

		// while?
		// TODO: need some offset for broadphase checks?
		physicsContext.haveIUpdated = false;
		if (physicsContext.m_lastUpdateTime + physicsContext.updateDuration < simulationTimeContext.tick_start_time)
		{
			physicsContext.m_lastUpdateTime = physicsContext.m_lastUpdateTime + physicsContext.updateDuration;
			physicsContext.haveIUpdated = true;
			// physicsContext.m_lastUpdateTime = simulationTimeContext.tick_start_time;
			 
			auto const simulationTimeStep = std::chrono::duration<float>(physicsContext.updateDuration / physicsContext.updateStepCount).count();

			std::vector<broadphase_result> broadphaseResults;

			for (auto [carEntity, carPosition, carRotation, carLinearVelocity, carAngularVelocityLocal, carCollider] : m_carColliderEntities.get().each())
			{
				auto const carTransform = aoest::combine4x3(carPosition, carRotation);

				// 1. Broadphase
				auto const carBounds = compute_car_bounds(carTransform, carCollider);
				// TODO: don't hard code search bounds extension, instead use max velocity or such.
				// also this could be error prone as it needs to include rotation.
				auto const searchBounds = aabb{ carBounds.min - glm::vec3{3.0f}, carBounds.max + glm::vec3{3.0f} };
				// TODO: this array could just be cleared to not reallocate for every car.
				std::vector<broadphase_result> broadphaseResults;
				for (auto [staticEntity, staticPosition, staticRotation, staticCollider] : m_staticColliderEntities.get().each())
				{
					for (auto& staticPart : staticCollider.parts)
					{
						staticPart.debugTriangleIndices.clear();
					}

					if (!test_intersection(searchBounds, staticCollider.bounds))
					{
						continue;
					}

					auto const staticTransform = aoest::combine4x3(staticPosition, staticRotation);
					for (auto& staticPart : staticCollider.parts)
					{
						int32_t t = 0;
						for (auto const& staticTriangleLocal : staticPart.triangles)
						{
							auto const staticTriangle = triangle{
								staticTransform * glm::vec4{staticTriangleLocal.p0, 1.0f},
								staticTransform * glm::vec4{staticTriangleLocal.p1, 1.0f},
								staticTransform * glm::vec4{staticTriangleLocal.p2, 1.0f}
							};

							auto const staticTriangleBounds = compute_triangle_bounds(staticTriangle);
							if (!test_intersection(searchBounds, staticTriangleBounds))
							{
								++t;
								continue;
							}

							broadphaseResults.emplace_back(staticTriangle, staticPart.material);
							staticPart.debugTriangleIndices.push_back(t);
							++t;
						}
					}
				}

				// TMP debug:
				std::vector<std::optional<std::pair<glm::vec3, glm::vec3>>> debugChassisContacts;
				std::vector<std::optional<std::pair<glm::vec3, glm::vec3>>> debugRimContacts;
				std::vector<std::optional<std::pair<glm::vec3, glm::vec3>>> debugTireContacts;
				for (auto const& chassisPart : carCollider.chassisParts)
				{
					debugChassisContacts.push_back(std::nullopt);
				}
				for (auto const& wheel : carCollider.wheels)
				{
					debugRimContacts.push_back(std::nullopt);
					debugTireContacts.push_back(std::nullopt);
				}

				// 2. Narrowphase
				auto const stepDuration = std::chrono::duration<float>(physicsContext.updateDuration / physicsContext.updateStepCount).count();
				for (int32_t updateStep = 0; updateStep < physicsContext.updateStepCount; ++updateStep)
				{
					auto const initialState = rk4_state{ carPosition, carRotation, carLinearVelocity, carAngularVelocityLocal };
					auto const k1 = rk4_derivate(carCollider, initialState, broadphaseResults,
						debugChassisContacts, debugRimContacts, debugTireContacts);
					auto const k2 = rk4_derivate(carCollider, rk4_step(initialState, k1, stepDuration * 0.5f), broadphaseResults,
						debugChassisContacts, debugRimContacts, debugTireContacts);
					auto const k3 = rk4_derivate(carCollider, rk4_step(initialState, k2, stepDuration * 0.5f), broadphaseResults,
						debugChassisContacts, debugRimContacts, debugTireContacts);
					auto const k4 = rk4_derivate(carCollider, rk4_step(initialState, k3, stepDuration), broadphaseResults,
						debugChassisContacts, debugRimContacts, debugTireContacts);

					carPosition += (simulationTimeStep / 6.0f) * (k1.position + 2.0f * k2.position + 2.0f * k3.position + k4.position);
					carRotation = glm::normalize(carRotation + (simulationTimeStep / 6.0f) * (k1.rotation + 2.0f * k2.rotation + 2.0f * k3.rotation + k4.rotation));
					carLinearVelocity += (simulationTimeStep / 6.0f) * (k1.linearVelocity + 2.0f * k2.linearVelocity + 2.0f * k3.linearVelocity + k4.linearVelocity);
					carAngularVelocityLocal += (simulationTimeStep / 6.0f) * (k1.angularVelocityLocal + 2.0f * k2.angularVelocityLocal + 2.0f * k3.angularVelocityLocal + k4.angularVelocityLocal);
				}

				carCollider.force = glm::vec3{ 0.0f, -25.0f, 0.0f } * carCollider.mass;
				carCollider.torque = glm::vec3{ 0.0f };

				// TMP debug:
				auto isDebugContactTooOld = [&physicsContext](int64_t a_maxDurationInNs) {
					return [&physicsContext, &a_maxDurationInNs](debug_contact const& a_debugContact) {
						return (physicsContext.m_lastUpdateTime - a_debugContact.time).count() > a_maxDurationInNs;
					};
				};

				int32_t c = 0;
				for (auto& chassisPart : carCollider.chassisParts)
				{
					if (chassisPart.debugContacts.size() > 10 || (!chassisPart.debugContacts.empty() && isDebugContactTooOld(10'000'000'000)(chassisPart.debugContacts.front())))
					{
						chassisPart.debugContacts.erase(chassisPart.debugContacts.begin());
					}

					if (debugChassisContacts[c] != std::nullopt)
					{
						chassisPart.debugContacts.emplace_back(
							physicsContext.m_lastUpdateTime,
							debugChassisContacts[c]->first,
							debugChassisContacts[c]->second);
					}

					++c;
				}
				int32_t w = 0;
				for (auto& wheel : carCollider.wheels)
				{
					if (wheel.debugRimContacts.size() > 10 || (!wheel.debugRimContacts.empty() && isDebugContactTooOld(10'000'000'000)(wheel.debugRimContacts.front())))
					{
						wheel.debugRimContacts.erase(wheel.debugRimContacts.begin());
					}
					if (!wheel.debugTireContacts.empty() && isDebugContactTooOld(15'000'000)(wheel.debugTireContacts.front()))
					{
						wheel.debugTireContacts.erase(wheel.debugTireContacts.begin());
					}

					if (debugRimContacts[w] != std::nullopt)
					{
						wheel.debugRimContacts.emplace_back(
							physicsContext.m_lastUpdateTime,
							debugRimContacts[w]->first,
							debugRimContacts[w]->second);
					}
					else if (debugTireContacts[w] != std::nullopt)
					{
						wheel.debugTireContacts.emplace_back(
							physicsContext.m_lastUpdateTime,
							debugTireContacts[w]->first,
							debugTireContacts[w]->second);
					}
					++w;
				}
			}
		}
	}
#pragma optimize("", on)
}
