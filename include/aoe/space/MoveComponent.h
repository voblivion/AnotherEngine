#pragma once

#include <aoe/core/ecs/Component.h>
#include <aoe/space/Export.h>
#include <aoe/space/Vector.h>

namespace aoe
{
	namespace space
	{
		struct AOE_SPACE_API MoveComponent final
			: public ecs::ComponentDefaultImpl<MoveComponent>
		{
			// Attributes
			Vector3 m_direction{};
			Vector3 m_rotation{};
		};
	}
}