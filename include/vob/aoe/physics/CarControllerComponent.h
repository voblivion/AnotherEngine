#pragma once

#include <vob/aoe/input/InputBindings.h>

#include <glm/glm.hpp>

#include <array>


namespace vob::aoeph
{
	struct CarControllerComponent
	{
		struct Wheel
		{
			float steeringFactor = 0.0f;
			float tireFriction = 2000.0f;

			float steeringAngle = 0.0f;
		};

		glm::vec3 gravity = glm::vec3{ 0.0f, -25.0f, 0.0f };
		
		float tireFriction = 2000.0f;
		float enginePower = 3000.0f;
		float brakePower = 12000.0f;
		float downForceStrength = 10.0f;
		float pitchRollAirControl = 8000.0f;
		float engineFriction = 100.0f;
		float airFriction = 50.0f;
		float brakeAirFriction = 250.0f;

		float minTurnSpeed = 0.0f;
		float maxTurnSpeed = 150.0f;

		std::array<Wheel, 4> wheels;

		aoein::InputBindings::SwitchId forwardInputId = aoein::InputBindings::kInvalidAxisId;
		aoein::InputBindings::SwitchId backwardInputId = aoein::InputBindings::kInvalidAxisId;
		aoein::InputBindings::AxisId steeringInputId = aoein::InputBindings::kInvalidSwitchId;
	};
}
