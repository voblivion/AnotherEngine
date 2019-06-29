#pragma once
#include <aoe/core/ecs/Component.h>
#include "aoe/core/ecs/EntityId.h"


namespace aoe
{
	namespace common
	{
		struct DirectorComponent final
			: public ecs::ComponentDefaultImpl<DirectorComponent>
		{
			ecs::EntityId m_currentCameraman{ ecs::g_invalidEntityId };
		};
	}
}
