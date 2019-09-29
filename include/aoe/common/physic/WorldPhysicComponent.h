#pragma once

#include <aoe/common/physic/ADynamicsWorldHolder.h>
#include <aoe/core/ecs/Component.h>

namespace aoe
{
	namespace common
	{
		struct WorldPhysicComponent final
			: public ecs::AComponent
		{
		public:
			explicit WorldPhysicComponent(
				sta::Clone<ADynamicsWorldHolder> a_dynamicsWorldHolder)
				: m_dynamicsWorldHolder{ std::move(a_dynamicsWorldHolder) }
			{}

			bool m_pause{ false };
			sta::Clone<ADynamicsWorldHolder> m_dynamicsWorldHolder;
		};
	}
}
