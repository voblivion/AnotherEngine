#pragma once

#include <aoe/common/Export.h>
#include <aoe/common/space/Vector.h>
#include <aoe/core/ecs/Component.h>

namespace aoe
{
	namespace common
	{
		struct AOE_COMMON_API MoveComponent final
			: public ecs::ComponentDefaultImpl<MoveComponent>
		{
			// Attributes
			Vector3 m_direction{};
			Vector3 m_rotation{};
		};
	}
}