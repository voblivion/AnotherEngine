#pragma once

#include <vob/aoe/common/time/Chrono.h>
#include <vob/misc/physics/measure.h>


namespace vob::aoe::common
{
	struct LifetimeComponent final
	{
		// Attributes
		misph::measure_time m_remainingTime{ 1.0f };
	};
}

namespace vob::misvi
{
	template <typename VisitorType, typename ThisType>
	requires std::is_same_v<std::remove_cvref_t<ThisType>, aoe::common::LifetimeComponent>
	bool accept(VisitorType& a_visitor, ThisType& a_this)
	{
		float remainingTime = 0.0f;
		a_visitor.visit(misvi::nvp("RemainingTime", remainingTime));
		a_this.m_remainingTime = misph::measure_time{ remainingTime };
		return true;
	}
}
