#pragma once

#include <entt/entt.hpp>
#include <glm/fwd.hpp>


namespace vob::aoest
{
	struct SoftFollowComponent
	{
		// Configuration
		entt::entity target = entt::null;
		glm::vec3 positionOffset = glm::vec3{ 0.0f };
		glm::vec3 aimOffset = glm::vec3{ 0.0f };
		float elasticity = 500.0f;
		float mass = 1.0f;
		float damping = 150.0f;
		
		// State
		glm::vec3 velocity = glm::vec3{ 0.0f };
		glm::vec3 prevTargetPosition = glm::vec3{ 0.0f };
	};
}
