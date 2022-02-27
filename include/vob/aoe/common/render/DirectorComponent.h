#pragma once

#include <vob/aoe/ecs/entity_id.h>


namespace vob::aoe::common
{
	struct DirectorComponent final
	{
		aoecs::entity_id m_currentCamera{ aoecs::k_invalid_entity_id };
	};
}
