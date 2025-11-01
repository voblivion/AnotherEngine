#include <vob/aoe/physics/vehicle_physics_system.h>

#include <vob/aoe/physics/maths.h>

#include <glm/glm.hpp>

#include "imgui.h"

namespace vob::aoeph
{
	vehicle_physics_system::vehicle_physics_system(aoeng::world_data_provider& a_wdp)
		: m_simulationTimeContext{ a_wdp }
		, m_physicsContext{ a_wdp }
		, m_debugMeshContext{ a_wdp }
		, m_blockerEntities{ a_wdp }
		, m_carColliderEntities{ a_wdp }
		, m_inputs{ a_wdp }
	{
	}

#pragma optimize("", off)
	void vehicle_physics_system::update() const
	{
		static auto k_gravity = -25.0f;
		static auto k_friction = 2000.0f;
		static auto k_power = 3000.0f;
		static auto k_break = 12000.0f;
		static auto k_downForceStrength = 10.0f;
		static auto k_pitchRollAirControl = 8000.0f;
		static auto k_engineFriction = 100.0f;
		static auto k_airFriction = 50.0f;
		static auto k_brakeAirFriction = 250.0f;

		static auto minTurnSpeed = 0.0f;
		static auto maxTurnSpeed = 150.0f;
		static auto minSpeedTurn = 0.698f / 2;
		static auto maxSpeedTurn = 0.436f / 2;

		static auto k_maxTurn = 0.0f;
		static auto k_speed = 0.0f;
		static auto k_isBraking = false;
		static auto k_isAccelerating = false;

		auto k_gasPedal = 0.0f;
		auto k_drivingWheelAngle = 0.0f;

		k_drivingWheelAngle = -m_inputs->gamepads[0].axes[aoein::gamepad::axis::LeftX].get_value();

		for (auto [entity, position, rotation, linearVelocity, angularVelocityLocal, carCollider] : m_carColliderEntities.get().each())
		{
			auto const barycenter = position + (rotation * carCollider.barycenter);

			auto const transform = aoest::combine(position, rotation);
			k_speed = glm::dot(rotation * glm::vec3{ 0.0f, 0.0f, -1.0f }, linearVelocity);

			k_isAccelerating = false;
			k_isBraking = false;

			if (m_inputs->gamepads[0].buttons[aoein::gamepad::button::A].is_pressed())
			{
				if (k_speed >= 0.0f)
				{
					k_isAccelerating = true;
				}
				else
				{
					k_isBraking = true;
				}
			}
			if (m_inputs->gamepads[0].buttons[aoein::gamepad::button::B].is_pressed())
			{
				if (k_speed <= 0.0f)
				{
					k_isAccelerating = true;
				}
				else
				{
					k_isBraking = true;
				}
			}

			k_maxTurn = static_cast<float>(0.52 * std::exp(0.000163386 * k_speed * k_speed - 0.04697899 * std::abs(k_speed)));

			// k_maxTurn = std::min(std::abs(k_speed), 150.0f) / maxTurnSpeed * (maxSpeedTurn - minSpeedTurn) + minSpeedTurn;
			auto const turn = k_drivingWheelAngle * k_maxTurn;

			if (m_physicsContext.get().just_updated)
			{
				carCollider.force = glm::vec3{ 0.0f, k_gravity, 0.0f } *carCollider.mass;
				carCollider.torque = glm::vec3{ 0.0f };

				carCollider.debugUpdateForces.emplace_back(position, glm::vec3{ 0.0f, k_gravity, 0.0f } *carCollider.mass);
			}

			bool isAnyWheelGrounded = false;
			for (auto& wheel : carCollider.wheels)
			{
				isAnyWheelGrounded = isAnyWheelGrounded || wheel.isTireGrounded;
			}

			if (m_physicsContext.get().just_updated)
			{
				// down force
				if (isAnyWheelGrounded)
				{
					auto const forwardVelocity = glm::dot(linearVelocity, rotation * glm::vec3{ 0.0f, 0.0f, -1.0f });
					auto const down = rotation * glm::vec3{ 0.0f, -1.0f, 0.0f };
					carCollider.force += forwardVelocity * k_downForceStrength * down;
				}
				else
				{
					if (k_isBraking)
					{
						carCollider.force -= k_brakeAirFriction * linearVelocity;
					}
					else if (!k_isAccelerating)
					{
						carCollider.force -= k_airFriction * linearVelocity;
					}
				}

				// pitch air control
				if (!isAnyWheelGrounded && k_isBraking)
				{
					auto const pitchRollVelocity = angularVelocityLocal * glm::vec3{ 1.0f, 0.0f, 1.0f };
					auto const pitchRollAirControlForceLocal = -k_pitchRollAirControl * pitchRollVelocity;
					carCollider.torque = rotation * pitchRollAirControlForceLocal;
				}

				if (isAnyWheelGrounded && !k_isAccelerating && !k_isBraking)
				{
					auto const velocity = glm::dot(linearVelocity, rotation * glm::vec3{ 0.0f, 0.0f, -1.0f });
					auto const dir = velocity > 0.0f ? rotation * glm::vec3{ 0.0f, 0.0f, -1.0f } : rotation * glm::vec3{ 0.0f, 0.0f, 1.0f };
					carCollider.force -= k_engineFriction * std::min(10.0f, std::abs(velocity)) * dir;
				}
			}

			for (auto& wheel : carCollider.wheels)
			{
				auto const wheelAttachmentPosition = aoest::apply(transform, wheel.attachPosition);
				auto const wheelAttachmentRotation = rotation * wheel.rotation;

				auto const wheelUp = wheelAttachmentRotation * glm::vec3{ 0.0f, 1.0f, 0.0f };
				
				auto const wheelTurn = wheel.turnFactor * turn;
				auto const wheelForward = wheelAttachmentRotation * glm::angleAxis(wheelTurn, glm::vec3{ 0.0f, 1.0f, 0.0f }) * glm::vec3{ 0.0f, 0.0f, -1.0f };
				auto const wheelRight = glm::cross(wheelForward, wheelUp);

				// debug

				if (m_physicsContext.get().just_updated)
				{
					wheel.turn = wheelTurn;
				}

				// physically accurater: if (wheel.isTireGrounded)
				if (isAnyWheelGrounded)
				{
					// physically accurater: auto const lever = wheel.groundPosition - barycenter;
					auto const groundPosition = !wheel.isTireGrounded
						? wheelAttachmentPosition - wheelUp * (wheel.suspensionLength + wheel.radiuses.y)
						: wheel.groundPosition;
					auto const lever = groundPosition - barycenter;
					auto const contactVelocity = linearVelocity + glm::cross(rotation * angularVelocityLocal, lever);
					auto const contactVelocityForward = glm::dot(contactVelocity, wheelForward) * wheelForward;
					auto const contactVelocityRight = glm::dot(contactVelocity, wheelRight) * wheelRight;
					auto const frictionForce = -k_friction * contactVelocityRight;
					
					auto const engineForce = k_isAccelerating ? (k_speed > 0.0f ? 1.0f : -1.0f) * k_power * wheelForward : glm::vec3{ 0.0f };
					auto const breakForce = k_isBraking ? (k_speed > 0.0f ? -1.0f : 1.0f) * k_break * wheelForward : glm::vec3{ 0.0f };

					if (m_physicsContext.get().just_updated)
					{
						carCollider.force += frictionForce + engineForce + breakForce;
						carCollider.torque += glm::cross(lever, frictionForce + engineForce + breakForce);

						carCollider.debugUpdateForces.emplace_back(wheel.groundPosition, engineForce + breakForce);
						carCollider.debugUpdateForces.emplace_back(wheel.groundPosition, frictionForce);
					}
				}
			}
		}

		ImGui::Begin("Vehicle");
		ImGui::InputFloat("Gravity", &k_gravity);
		ImGui::InputFloat("Min Turn Speed", &minTurnSpeed);
		ImGui::InputFloat("Max Turn Speed", &maxTurnSpeed);
		ImGui::InputFloat("Min Speed Turn", &minSpeedTurn);
		ImGui::InputFloat("Max Speed Turn", &maxSpeedTurn);
		ImGui::InputFloat("Power", &k_power);
		ImGui::InputFloat("Break", &k_break);
		ImGui::InputFloat("Down Force Strength", &k_downForceStrength);
		ImGui::InputFloat("Pitch/Roll Air Control", &k_pitchRollAirControl);
		ImGui::InputFloat("Engine Friction", &k_engineFriction);
		ImGui::InputFloat("Friction", &k_friction);
		ImGui::InputFloat("Air Friction", &k_airFriction);
		ImGui::InputFloat("Brake Air Friction", &k_brakeAirFriction);
		ImGui::BeginDisabled();
		ImGui::Checkbox("Is Accelerating", &k_isAccelerating);
		ImGui::Checkbox("Is Braking", &k_isBraking);
		ImGui::InputFloat("Speed", &k_speed);
		ImGui::InputFloat("Max Turn", &k_maxTurn);
		ImGui::InputFloat("Wheel Driving Angle", &k_drivingWheelAngle);
		ImGui::EndDisabled();
		ImGui::End();
	}
}
