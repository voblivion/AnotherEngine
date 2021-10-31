#pragma once

#include <vob/sta/measure.h>

#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/common/time/Chrono.h>
#include <vob/sta/physic_measure.h>

using namespace vob::sta::literals;

namespace vob::aoe::common
{
	struct WorldTimeComponent final
		: ecs::AComponent
	{
		// Attributes
		TimePoint m_frameStartTime;
		Duration m_frameDuration{ 0.0f };
		sta::time_measure<float> m_elapsedTime = 0.0_s;
	};
}
