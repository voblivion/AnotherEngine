#pragma once

#include <vob/aoe/engine/world_data_provider.h>

#include <glm/glm.hpp>

#include <vector>


namespace vob::aoeac
{
	struct actor_component
	{
		// Movement
		struct waypoint
		{
			glm::vec3 m_position;
			bool m_isJump = false;
		};
		std::vector<waypoint> m_waypoints;

		// actions ?
		std::vector<aoeng::entity> m_actions;
	};
}

namespace vob::misvi
{
	template <typename VisitorType, typename ThisType>
	requires std::is_same_v<std::remove_cvref_t<ThisType>, aoeac::actor_component>
	bool accept(VisitorType& a_visitor, ThisType& a_this) { return true; }
}
