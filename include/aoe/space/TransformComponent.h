#pragma once

#include <aoe/core/ecs/Component.h>
#include <aoe/space/Export.h>
#include <aoe/space/Vector.h>

namespace aoe
{
	namespace space
	{
		struct AOE_SPACE_API TransformComponent final
			: public ecs::ComponentDefaultImpl<TransformComponent>
		{
			Vector3 m_position;
			Vector3 m_orientation;
		};
	}
}