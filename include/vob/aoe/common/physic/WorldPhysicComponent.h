#pragma once

#include <vob/aoe/common/physic/ADynamicsWorldHolder.h>
#include <vob/aoe/core/ecs/Component.h>

namespace vob::aoe::common
{
	class WorldPhysicComponent final
		: public ecs::AComponent
	{
	public:
		explicit WorldPhysicComponent(
			type::Cloneable<ADynamicsWorldHolder> a_dynamicsWorldHolder
		)
			: m_dynamicsWorldHolder{ std::move(a_dynamicsWorldHolder) }
		{}

		bool m_pause{ false };
		type::Cloneable<ADynamicsWorldHolder> m_dynamicsWorldHolder;
	};
}
