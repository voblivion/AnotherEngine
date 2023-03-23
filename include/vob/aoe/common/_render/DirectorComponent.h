#pragma once

#include <vob/aoe/ecs/_entity_id.h>


namespace vob::aoe::common
{
	struct DirectorComponent final
	{
		_aoecs::entity_id m_currentCamera;
	};
}
