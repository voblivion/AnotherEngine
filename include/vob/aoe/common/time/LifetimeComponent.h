#pragma once

#include <vob/aoe/common/time/Chrono.h>
#include <vob/aoe/core/ecs/Component.h>


namespace vob::aoe::common
{
	struct LifetimeComponent final
		: public ecs::AComponent
	{
		// Attributes
		float m_remainingTime{ 1.0f };
	};
}

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ThisType>
	visitIfType<common::LifetimeComponent, ThisType> accept(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(vis::nvp("RemainingTime", a_this.m_remainingTime));
	}
}
