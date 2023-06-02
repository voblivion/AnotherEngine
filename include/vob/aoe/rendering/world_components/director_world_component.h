#pragma once

#include <vob/aoe/ecs/entity_list.h>

#include <vob/aoe/engine/world.h>


namespace vob::aoegl
{
	struct director_world_component
	{
		aoeng::entity m_activeCamera;
		aoecs::entity_id m_activeCameraId;
	};
}
