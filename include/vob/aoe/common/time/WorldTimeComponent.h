#pragma once

#include <vob/aoe/ecs/Component.h>
#include <vob/aoe/common/time/Chrono.h>

#include <vob/misc/physics/measure.h>
#include <vob/misc/physics/measure_literals.h>


using namespace vob::misph::literals;

namespace vob::aoe::common
{
	struct WorldTimeComponent final
		: aoecs::AComponent
	{
		// Attributes
		TimePoint m_frameStartTime;
		Duration m_frameDuration{ 0.0f };
		misph::measure_time m_elapsedTime = 0.0_s;
	};
}
