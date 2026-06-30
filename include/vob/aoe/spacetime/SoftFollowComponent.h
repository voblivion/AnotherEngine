#pragma once

#include <entt/entt.hpp>
#include <glm/fwd.hpp>


namespace vob::aoest
{
	struct SoftFollowComponent
	{
		// Configuration
		entt::entity target = entt::null;

		float positionDistance = 10.0f;
		float positionPitch = 0.5f;

		float lookAheadDistance = 10.0f;

		// TODO: runtime makes it a mix of world's up and ground's up
		glm::vec3 referenceUp = glm::vec3{ 0.0f, 1.0f, 0.0f };
		glm::vec3 currentDir = glm::vec3{ 0.0f };
	};
}
