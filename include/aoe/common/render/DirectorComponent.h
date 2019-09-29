#pragma once
#include <aoe/core/ecs/Component.h>
#include "aoe/core/ecs/EntityId.h"


namespace aoe
{
	namespace common
	{
		struct DirectorComponent final
			: public ecs::AComponent
		{
			ecs::EntityId m_currentCamera{ ecs::g_invalidEntityId };
		};
	}
}
