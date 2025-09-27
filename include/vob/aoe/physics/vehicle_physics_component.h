#pragma once

#include <vob/aoe/input/bindings.h>

#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>


namespace vob::aoeph
{
	struct wheel
	{
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 radiuses = glm::vec3{ 1.0f };

		float turnFactor = 1.0f;

		float suspensionMaxLength = 0.2f;
		float suspensionEllasticity = 6000.0f;
		float suspensionDamper = 4000.0f;
		float tireFriction = 0.05f;
	};

	struct vehicle_physics_component
	{
		std::vector<wheel> wheels;
	};

	struct vehicle_controller_context
	{
		aoein::bindings::axis_id m_turn = 0;
		aoein::bindings::switch_id m_forward = 0;
		aoein::bindings::switch_id m_reverse = 0;
		aoein::bindings::switch_id m_respawn = 0;
	};
}
