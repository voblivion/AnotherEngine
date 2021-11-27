#pragma once
#include <vob/aoe/ecs/Component.h>
#include <vob/aoe/ecs/EntityId.h>


namespace vob::aoe::common
{
	struct DirectorComponent final
		: public aoecs::AComponent
	{
		aoecs::EntityId m_currentCamera{ aoecs::g_invalidEntityId };
	};
}
