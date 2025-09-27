#include <vob/aoe/physics/vehicle_physics_system.h>

#include <vob/aoe/physics/maths.h>

#include <glm/glm.hpp>

#include "imgui.h"

namespace vob::aoeph
{
	vehicle_physics_system::vehicle_physics_system(aoeng::world_data_provider& a_wdp)
		: m_simulationTimeContext{ a_wdp }
		, m_debugMeshContext{ a_wdp }
		, m_vehicleEntities{ a_wdp }
		, m_blockerEntities{ a_wdp }
		, m_carColliderEntities{ a_wdp }
		, m_inputs{ a_wdp }
	{
	}

	void vehicle_physics_system::update() const
	{
		std::array<float, 4> exts;
		static auto k_gravity = -25.0f;
		static auto k_ellasticity = 32'894.0f;
		static auto k_dampener = 5'735.0f;
		static auto k_length = 0.2f;
		static auto k_friction = 4000.0f;
		static auto k_power = 4000.0f;
		static auto k_maxTurn = 0.0f;
		static auto k_speed = 0.0f;

		auto k_gas = 0.0f;
		auto k_drivingWheelAngle = 0.0f;

		if (m_inputs->gamepads[0].buttons[aoein::gamepad::button::A].is_pressed())
		{
			k_gas += k_power;
		}
		if (m_inputs->gamepads[0].buttons[aoein::gamepad::button::B].is_pressed())
		{
			k_gas -= k_power;
		}
		k_drivingWheelAngle = -m_inputs->gamepads[0].axes[aoein::gamepad::axis::LeftX].get_value();

		aoeph::dynamic_body rrbb;
		aoeph::dynamic_body* rb = &rrbb;

		for (auto [entity, position, rotation, linearVelocity, angularVelocityLocal, carCollider] : m_carColliderEntities.get().each())
		{
			auto const barycenter = position + (rotation * carCollider.barycenter);

			carCollider.force = glm::vec3{ 0.0f, k_gravity, 0.0f } * carCollider.mass;
			carCollider.torque = glm::vec3{ 0.0f };

			auto const transform = aoest::combine(position, rotation);
			auto const forwardSpeed = glm::dot(rotation * glm::vec3{ 0.0f, 0.0f, -1.0f }, linearVelocity);
			auto const maxTurn = std::min(std::abs(forwardSpeed) * 3.60f, 150.0f) / 150.0f * (0.436f / 2 - 0.698f / 2) + 0.698f / 2;
			auto const turn = k_drivingWheelAngle * maxTurn;

			int w = 0;
			for (auto& wheel : carCollider.wheels)
			{
				exts[w++] = wheel.suspensionLength;
				auto const wheelAttachmentPosition = aoest::apply(transform, wheel.attachPosition);
				auto const wheelAttachmentRotation = rotation * wheel.rotation;

				auto wheelAttachmentTransform = aoest::combine(wheelAttachmentPosition, wheelAttachmentRotation);

				auto const wheelUp = wheelAttachmentRotation * glm::vec3{ 0.0f, 1.0f, 0.0f };
				
				auto const wheelTurn = wheel.turnFactor * turn;
				auto const wheelForward = wheelAttachmentRotation * glm::angleAxis(wheelTurn, glm::vec3{ 0.0f, 1.0f, 0.0f }) * glm::vec3{ 0.0f, 0.0f, -1.0f };
				auto const wheelRight = glm::cross(wheelForward, wheelUp);

				auto suspensionDisplacement = -wheel.suspensionLength * glm::vec3{ 0.0f, 1.0f, 0.0f };
				auto wheelPosition = wheelAttachmentPosition + suspensionDisplacement;

				if (wheel.isGrounded)
				{
					auto const lever = wheel.groundPosition - barycenter;
					auto const contactVelocity = linearVelocity + glm::cross(angularVelocityLocal, lever);
					auto const contactVelocityForward = glm::dot(contactVelocity, wheelForward) * wheelForward;
					auto const contactVelocityRight = glm::dot(contactVelocity, wheelRight) * wheelRight;
					auto const frictionForce = -k_friction * contactVelocityRight;

					auto const engineForce = k_gas * wheelForward;

					carCollider.force += frictionForce + engineForce;
					carCollider.torque += glm::cross(lever, frictionForce + engineForce);
				}
			}
		}

		ImGui::Begin("Vehicle");
		ImGui::InputFloat("0", &exts[0]);
		ImGui::InputFloat("1", &exts[1]);
		ImGui::InputFloat("2", &exts[2]);
		ImGui::InputFloat("3", &exts[3]);
		ImGui::InputFloat("Gravity", &k_gravity);
		ImGui::InputFloat("Ellasticity", &k_ellasticity);
		ImGui::InputFloat("Dampener", &k_dampener);
		ImGui::InputFloat("Length", &k_length);
		ImGui::InputFloat("Power", &k_power);
		ImGui::InputFloat("Friction", &k_friction);
		ImGui::InputFloat("Speed", &k_speed);
		ImGui::InputFloat("Max Turn", &k_maxTurn);
		ImGui::InputFloat("Wheel Driving Angle", &k_drivingWheelAngle);
		ImGui::InputFloat("Mass", &rb->mass);
		ImGui::InputFloat3("Barycenter", &rb->barycenter.x);
		ImGui::End();
	}
}
