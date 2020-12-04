#pragma once
#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/core/ecs/EntityId.h>


namespace vob::aoe::common
{
	struct DirectorComponent final
		: public ecs::AComponent
	{
		ecs::EntityId m_currentCamera{ ecs::g_invalidEntityId };
	};
}
