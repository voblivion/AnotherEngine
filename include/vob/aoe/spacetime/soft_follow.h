#pragma once

#include <vob/aoe/engine/world.h>

#include <glm/glm.hpp>


namespace vob::aoest
{
	struct soft_follow
	{
		aoeng::entity m_target = entt::null;
		glm::vec3 m_aimTarget = glm::vec3{ 0.0f };
		glm::vec3 m_posTarget = glm::vec3{ 0.0f };
		float m_maxDist = 100.0f;
		float m_smoothing = 0.9f;
	};
}
