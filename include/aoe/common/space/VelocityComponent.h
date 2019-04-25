#pragma once

#include <aoe/common/Export.h>
#include <aoe/common/space/Vector.h>
#include <aoe/core/ecs/Component.h>

namespace aoe
{
	namespace common
	{
		struct AOE_COMMON_API VelocityComponent final
			: public ecs::ComponentDefaultImpl<VelocityComponent>
		{
			// Attributes
			Vector3 m_linear{};
			Vector3 m_angular{};
		};
	}
}