#include <vob/aoe/physics/CarControllerSystem.h>

#include <glm/gtc/quaternion.hpp>

#include <cstdint>

#include "imgui.h"


#pragma optimize("", off)
namespace vob::aoeph
{
	void CarControllerSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_inputBindings.init(a_wdar);
		m_carEntities.init(a_wdar);
	}

	void CarControllerSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto const& inputBindings = m_inputBindings.get(a_wdap);

		for (auto [carEntity, position, rotation, carCollider, carControllerComponent] : m_carEntities.get(a_wdap).each())
		{
			auto const barycenter = position + rotation * carCollider.barycenterLocal;

			auto const forward = rotation * glm::vec3{ 0.0f, 0.0f, -1.0f };
			auto const down = rotation * glm::vec3{ 0.0f, -1.0f, 0.0f };
			auto /*const*/ speed = glm::dot(forward, carCollider.linearVelocity);

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

			auto /*const*/ maxWheelSteeringAngle = static_cast<float>(steering * 0.52 * std::exp(0.000163386 * speed * speed - 0.04697899 * std::abs(speed)));

			carCollider.force = carControllerComponent.gravity * carCollider.mass;
			carCollider.torque = glm::vec3{ 0.0f };
			auto isGrounded = false;

			for (auto const& wheel : carCollider.wheels)
			{
				isGrounded |= wheel.isGrounded;
			}

			if (isGrounded)
			{
				carCollider.force += speed * carControllerComponent.downForceStrength * down;
			}
			else
			{
				if (isDecelerating)
				{
					carCollider.force += -carControllerComponent.brakeAirFriction * carCollider.linearVelocity;
				}
				else if (!isAccelerating)
				{
					carCollider.force += -carControllerComponent.airFriction * carCollider.linearVelocity;
				}
			}

			if (!isGrounded && isDecelerating)
			{
				auto const pitchRollVelocityLocal = carCollider.angularVelocityLocal * glm::vec3{ 1.0f, 0.0f, 1.0f };
				carCollider.torque += -carControllerComponent.pitchRollAirControl * rotation * pitchRollVelocityLocal;
			}

			if (isGrounded && !isAccelerating && !isDecelerating && glm::length(carCollider.linearVelocity) > std::numeric_limits<float>::epsilon())
			{
				auto const movingDir = glm::normalize(carCollider.linearVelocity);
				carCollider.force += -carControllerComponent.engineFriction * std::min(10.0f, std::abs(speed)) * movingDir;
			}

			for (int32_t i = 0; i < 4; ++i)
			{
				auto const& wheelCollider = carCollider.wheels[i];
				auto const& wheelController = carControllerComponent.wheels[i];

				auto const suspensionAttachPosition = position + rotation * wheelCollider.suspensionAttachPosition;
				auto const wheelRotation = rotation * wheelCollider.rotation;

				auto const wheelUp = wheelRotation * glm::vec3{ 0.0f, 1.0f, 0.0f };
				auto const wheelSteeringAngle = maxWheelSteeringAngle * wheelController.steeringFactor;
				auto const wheelForward = wheelRotation * glm::angleAxis(wheelSteeringAngle, glm::vec3{ 0.0f, 1.0f, 0.0f }) * glm::vec3{ 0.0f, 0.0f, -1.0f };
				auto const wheelRight = glm::cross(wheelForward, wheelUp);

				if (isGrounded)
				{
					// TODO: this lever is wrong, either I handle grounded wheels only, or I do something wrong.
					auto const lever = suspensionAttachPosition - barycenter;
					auto const contactVelocity = carCollider.linearVelocity + glm::cross(rotation * carCollider.angularVelocityLocal, lever);
					auto const contactVelocityForward = glm::dot(contactVelocity, wheelForward) * wheelForward;
					auto const contactVelocityRight = glm::dot(contactVelocity, wheelRight) * wheelRight;
					
					auto const frictionForce = -wheelController.tireFriction * contactVelocityRight;
					auto const engineForce = isAccelerating
						? (speed >= 0.0f ? 1.0f : -1.0f) * carControllerComponent.enginePower * wheelForward : glm::vec3{ 0.0f };
					auto const brakeForce = isDecelerating
						? (speed >= 0.0f ? -1.0f : 1.0f) * carControllerComponent.brakePower * wheelForward : glm::vec3{ 0.0f };

					carCollider.force += frictionForce + engineForce + brakeForce;
					carCollider.torque += glm::cross(lever, frictionForce + engineForce + brakeForce);
				}
			}
		}
	}
}
