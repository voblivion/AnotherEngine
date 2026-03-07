#include <vob/aoe/physics/CarControllerSystem.h>

#include <vob/misc/std/ignorable_assert.h>

#include <imgui.h>

#include <glm/gtc/quaternion.hpp>

#include <cstdint>


namespace vob::aoeph
{
	void CarControllerSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_carEntities.init(a_wdar);
	}

	void CarControllerSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto& fixedRateTimeContext = m_fixedRateTimeContext.get(a_wdap);
		for (auto [carEntity, position, rotation, carColliderCmp, carControllerComponent] : m_carEntities.get(a_wdap).each())
		{
			for (auto const eventId : m_gameInputCtx.get(a_wdap).getEvents())
			{
				if (eventId == carControllerComponent.respawnInputEventId)
				{
					position = carControllerComponent.respawnPosition;
					rotation = carControllerComponent.respawnRotation;
					carColliderCmp.linearVelocity = glm::vec3{ 0.0f };
					carColliderCmp.angularVelocityLocal = glm::vec3{ 0.0f };
					carColliderCmp.force = glm::vec3{ 0.0f };
					carColliderCmp.torque = glm::vec3{ 0.0f };
					for (auto& wheel : carColliderCmp.wheels)
					{
						wheel.suspensionLength = 0.0f;
						wheel.suspensionVelocity = 0.0f;
					}
					for (auto& wheel : carControllerComponent.wheels)
					{
						wheel.steeringAngle = 0.0f;
					}
				}
				else if (eventId == carControllerComponent.instantBrakeInputEventId)
				{
					carColliderCmp.linearVelocity = glm::vec3{ 0.0f };
					carColliderCmp.angularVelocityLocal = glm::vec3{ 0.0f };
				}
				else if (eventId == carControllerComponent.stepInputEventId)
				{
					fixedRateTimeContext.debugRemainingTickCount = 1;
				}
				else if (eventId == carControllerComponent.playInputEventId)
				{
					fixedRateTimeContext.debugRemainingTickCount = -1;
				}
				else if (eventId == carControllerComponent.revertInputEventId)
				{
					fixedRateTimeContext.debugRemainingTickCount = 0;

					position = carControllerComponent.previousState.position;
					rotation = carControllerComponent.previousState.rotation;
					carColliderCmp.linearVelocity = carControllerComponent.previousState.linearVelocity;
					carColliderCmp.angularVelocityLocal = carControllerComponent.previousState.angularVelocityLocal;

					for (int32_t w = 0; w < 4; ++w)
					{
						carColliderCmp.wheels[w].suspensionLength = carControllerComponent.previousState.wheels[w].suspensionLength;
						carColliderCmp.wheels[w].suspensionVelocity = carControllerComponent.previousState.wheels[w].suspensionVelocity;
						carColliderCmp.wheels[w].isGrounded = carControllerComponent.previousState.wheels[w].isGrounded;
						carColliderCmp.wheels[w].groundNormal = carControllerComponent.previousState.wheels[w].groundNormal;
						carColliderCmp.wheels[w].groundPoint = carControllerComponent.previousState.wheels[w].groundPoint;
						carColliderCmp.wheels[w].tireGroundedPoint = carControllerComponent.previousState.wheels[w].tireGroundedPoint;
						carColliderCmp.wheels[w].groundMaterial = carControllerComponent.previousState.wheels[w].groundMaterial;
						for (auto& step : carColliderCmp.wheels[w].chassisContacts)
						{
							step[0].clear();
							step[1].clear();
							step[2].clear();
							step[3].clear();
						}
						for (auto& step : carColliderCmp.wheels[w].contacts)
						{
							step[0].clear();
							step[1].clear();
							step[2].clear();
							step[3].clear();
						}
					}

					for (auto& chassisPart : carColliderCmp.chassisParts)
					{
						for (auto& step : chassisPart.contacts)
						{
							step[0].clear();
							step[1].clear();
							step[2].clear();
							step[3].clear();
						}
					}
				}
			}

			// TODO: better implement
			if (fixedRateTimeContext.debugRemainingTickCount != 0)
			{
				carControllerComponent.previousState.position = position;
				carControllerComponent.previousState.rotation = rotation;
				carControllerComponent.previousState.linearVelocity = carColliderCmp.linearVelocity;
				carControllerComponent.previousState.angularVelocityLocal = carColliderCmp.angularVelocityLocal;

				for (int32_t w = 0; w < 4; ++w)
				{
					carControllerComponent.previousState.wheels[w].suspensionLength = carColliderCmp.wheels[w].suspensionLength;
					carControllerComponent.previousState.wheels[w].suspensionVelocity = carColliderCmp.wheels[w].suspensionVelocity;
					carControllerComponent.previousState.wheels[w].isGrounded = carColliderCmp.wheels[w].isGrounded;
					carControllerComponent.previousState.wheels[w].groundNormal = carColliderCmp.wheels[w].groundNormal;
					carControllerComponent.previousState.wheels[w].groundPoint = carColliderCmp.wheels[w].groundPoint;
					carControllerComponent.previousState.wheels[w].tireGroundedPoint = carColliderCmp.wheels[w].tireGroundedPoint;
					carControllerComponent.previousState.wheels[w].groundMaterial = carColliderCmp.wheels[w].groundMaterial;
				}
			}

			auto const barycenter = position + rotation * carColliderCmp.barycenterLocal;

			auto const forward = rotation * glm::vec3{ 0.0f, 0.0f, -1.0f };
			auto const down = rotation * glm::vec3{ 0.0f, -1.0f, 0.0f };
			auto /*const*/ speed = glm::dot(forward, carColliderCmp.linearVelocity);

			auto isAccelerating = false;
			auto isDecelerating = false;
			auto steering = 0.0f;

			auto const forwardInput = m_gameInputCtx.get(a_wdap).getValue(carControllerComponent.forwardInputValueId);
			auto const backwardInput = m_gameInputCtx.get(a_wdap).getValue(carControllerComponent.backwardInputValueId);

			if (forwardInput > 0.0f)
			{
				if (speed >= 0.0f)
				{
					isAccelerating = true;
				}
				else
				{
					isDecelerating = true;
				}
			}

			if (backwardInput > 0.0f)
			{
				if (speed < 0.0f)
				{
					isAccelerating = true;
				}
				else
				{
					isDecelerating = true;
				}
			}

			steering = -m_gameInputCtx.get(a_wdap).getValue(carControllerComponent.steeringInputValueId);

			// BEGIN DEBUGGING
			//isAccelerating = true;
			//isDecelerating = false;
			//steering = std::pow(static_cast<float>(fixedRateTimeContext.tickIndex) / 1350.0f, 11.0f);
			// END DEBUGGING

			auto /*const*/ maxWheelSteeringAngle = static_cast<float>(steering * 0.52 * std::exp(0.000163386 * speed * speed - 0.04697899 * std::abs(speed)));

			carColliderCmp.force = carControllerComponent.gravity * carColliderCmp.mass;
			carColliderCmp.torque = glm::vec3{ 0.0f };
			auto isGrounded = false;

			for (auto const& wheel : carColliderCmp.wheels)
			{
				isGrounded |= wheel.isGrounded;
			}

			if (isGrounded)
			{
				carColliderCmp.force += speed * carControllerComponent.downForceStrength * down;
			}
			else
			{
				if (isDecelerating)
				{
					carColliderCmp.force += -carControllerComponent.brakeAirFriction * carColliderCmp.linearVelocity;
				}
				else if (!isAccelerating)
				{
					carColliderCmp.force += -carControllerComponent.airFriction * carColliderCmp.linearVelocity;
				}
			}

			if (!isGrounded && isDecelerating)
			{
				auto const pitchRollVelocityLocal = carColliderCmp.angularVelocityLocal * glm::vec3{ 1.0f, 0.0f, 1.0f };
				carColliderCmp.torque += -carControllerComponent.pitchRollAirControl * rotation * pitchRollVelocityLocal;
				// ignorable_assert(glm::length(carColliderCmp.torque) < 1'000'000.0f);
			}

			if (isGrounded && !isAccelerating && !isDecelerating && glm::length(carColliderCmp.linearVelocity) > std::numeric_limits<float>::epsilon())
			{
				auto const movingDir = glm::normalize(carColliderCmp.linearVelocity);
				carColliderCmp.force += -carControllerComponent.engineFriction * std::min(10.0f, std::abs(speed)) * movingDir;
			}

			for (int32_t i = 0; i < 4; ++i)
			{
				auto& wheelCollider = carColliderCmp.wheels[i];
				auto& wheelController = carControllerComponent.wheels[i];

				auto const suspensionAttachPosition = position + rotation * wheelCollider.suspensionAttachPosition;
				auto const wheelRotation = rotation * wheelCollider.rotation;

				auto const wheelUp = wheelRotation * glm::vec3{ 0.0f, 1.0f, 0.0f };
				auto const wheelSteeringAngle = maxWheelSteeringAngle * wheelController.steeringFactor;
				// TODO: not part of controller!
				wheelController.steeringAngle = wheelSteeringAngle;
				auto const wheelForward = wheelRotation * glm::angleAxis(wheelSteeringAngle, glm::vec3{ 0.0f, 1.0f, 0.0f }) * glm::vec3{ 0.0f, 0.0f, -1.0f };
				auto const wheelRight = glm::cross(wheelForward, wheelUp);

				if (isGrounded) // wheelCollider.isGrounded
				{
					// TODO: yes, can be negative, trying something so I can drive turtle
					auto const traction = glm::dot(wheelUp, wheelCollider.groundNormal);
					// TODO: this lever is wrong, either I handle grounded wheels only, or I do something wrong.
					auto const wheelPoint = -(wheelCollider.suspensionLength + glm::dot(wheelUp, wheelCollider.radiuses)) * wheelUp + suspensionAttachPosition;
					auto const lever = wheelPoint - barycenter; // used to be suspensionAttachPosition - barycenter
					auto const contactVelocity = carColliderCmp.linearVelocity + glm::cross(rotation * carColliderCmp.angularVelocityLocal, lever);
					auto const contactVelocityForward = glm::dot(contactVelocity, wheelForward) * wheelForward;
					auto const contactVelocityRight = glm::dot(contactVelocity, wheelRight) * wheelRight;
					
					auto const frictionForce = -wheelController.tireFriction * contactVelocityRight;
					auto const engineForce = isAccelerating
						? (speed >= 0.0f ? 1.0f : -1.0f) * carControllerComponent.enginePower * wheelForward * traction : glm::vec3{ 0.0f };
					auto const brakeForce = isDecelerating
						? (speed >= 0.0f ? -1.0f : 1.0f) * carControllerComponent.brakePower * wheelForward * traction : glm::vec3{ 0.0f };

					carColliderCmp.force += frictionForce + engineForce + brakeForce;
					carColliderCmp.torque += glm::cross(lever, frictionForce + engineForce + brakeForce);
					// ignorable_assert(glm::length(carColliderCmp.torque) < 1'000'000.0f);
				}
			}
		}
	}
}
