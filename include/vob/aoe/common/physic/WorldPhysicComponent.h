#pragma once

#include <vob/aoe/common/physic/ADynamicsWorldHolder.h>
#include <vob/aoe/ecs/Component.h>

namespace vob::aoe::common
{
	class WorldPhysicComponent final
		: public aoecs::AComponent
	{
	public:
		explicit WorldPhysicComponent(
			type::dynamic_type_clone<ADynamicsWorldHolder> a_dynamicsWorldHolder
		)
			: m_dynamicsWorldHolder{ std::move(a_dynamicsWorldHolder) }
		{}

		bool m_pause = false;
		bool m_displayDebug = false;
		type::dynamic_type_clone<ADynamicsWorldHolder> m_dynamicsWorldHolder;
	};
}
