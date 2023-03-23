#pragma once

#include <vob/aoe/ecs/entity_list.h>

namespace vob::aoegl
{
	struct director_world_component
	{
		aoecs::entity_id m_activeCamera;
	};
}
