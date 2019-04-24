#pragma once

#include <aoe/core/ecs/Component.h>
#include <aoe/common/Export.h>
#include <aoe/common/space/Vector.h>

namespace aoe
{
	namespace common
	{
		struct AOE_COMMON_API TransformComponent final
			: public ecs::ComponentDefaultImpl<TransformComponent>
		{
			Vector3 m_position;
			Vector3 m_orientation;
		};
	}
}