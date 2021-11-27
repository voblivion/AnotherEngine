#pragma once

#include <vob/aoe/common/time/Chrono.h>
#include <vob/aoe/ecs/Component.h>
#include <vob/misc/physics/measure.h>


namespace vob::aoe::common
{
	struct LifetimeComponent final
		: public aoecs::AComponent
	{
		// Attributes
		misph::measure_time m_remainingTime{ 1.0f };
	};
}

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ThisType>
	visitIfType<common::LifetimeComponent, ThisType> accept(VisitorType& a_visitor, ThisType& a_this)
	{
		float remainingTime = 0.0f;
		a_visitor.visit(vis::nvp("RemainingTime", remainingTime));
		a_this.m_remainingTime = misph::measure_time{ remainingTime };
	}
}
