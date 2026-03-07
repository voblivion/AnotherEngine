#pragma once

#include <vob/aoe/physics/Material.h>

#include <vob/aoe/input/InputBindings.h>
#include <vob/aoe/input/GameInput.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <array>


namespace vob::aoeph
{
	struct CarState
	{
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 linearVelocity;
		glm::vec3 angularVelocityLocal;

		struct Wheel
		{
			float suspensionLength = 0.0f;
			float suspensionVelocity = 0.0f;
			bool isGrounded = false;
			glm::vec3 groundNormal = glm::vec3{ 0.0f };
			glm::vec3 groundPoint = glm::vec3{ 0.0f };
			glm::vec3 tireGroundedPoint = glm::vec3{ 0.0f };
			Material groundMaterial;
		};

		std::array<Wheel, 4> wheels;
	};

	struct CarControllerComponent
	{
		struct Wheel
		{
			float steeringFactor = 0.0f;
			float tireFriction = 4000.0f;

			float steeringAngle = 0.0f;
		};

		glm::vec3 respawnPosition = glm::vec3{ 0.0f };
		glm::quat respawnRotation = glm::quat{1.0f, 0.0f, 0.0f, 0.0f};
		glm::vec3 gravity = glm::vec3{ 0.0f, -25.0f, 0.0f };
		
		float tireFriction = 1000.0f;
		float enginePower = 3000.0f;
		float brakePower = 6000.0f;
		float downForceStrength = 10.0f;
		float pitchRollAirControl = 1.0f;
		float engineFriction = 100.0f;
		float airFriction = 50.0f;
		float brakeAirFriction = 250.0f;

		float minTurnSpeed = 0.0f;
		float maxTurnSpeed = 150.0f;

		CarState previousState;

		std::array<Wheel, 4> wheels;

		aoein::GameInputValueId forwardInputValueId = {};
		aoein::GameInputValueId backwardInputValueId = {};
		aoein::GameInputValueId steeringInputValueId = {};
		aoein::GameInputEventId respawnInputEventId = {};
		aoein::GameInputEventId instantBrakeInputEventId = {};

		aoein::GameInputEventId stepInputEventId = {};
		aoein::GameInputEventId playInputEventId = {};
		aoein::GameInputEventId revertInputEventId = {};
	};
}
