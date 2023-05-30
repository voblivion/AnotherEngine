#pragma once

#include <vob/aoe/ecs/entity_list.h>


namespace vob::aoe::common
{
	struct DirectorComponent final
	{
		aoecs::entity_id m_currentCamera;
	};
}
