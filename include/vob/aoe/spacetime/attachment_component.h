#pragma once

#include <vob/aoe/engine/world_data.h>

#include <vob/misc/std/message_macros.h>

#include <glm/glm.hpp>


namespace vob::aoest
{
	struct attachment_component
	{
#pragma message(VOB_MISTD_TODO "bone?")

		bool m_needsUpdate = false;
		aoeng::entity m_parent = entt::tombstone;
		glm::mat4 m_localTransform = glm::mat4{ 1.0f };
	};
}