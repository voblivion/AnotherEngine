#pragma once

#include <vob/misc/physics/measure.h>
#include <vob/misc/physics/measure_literals.h>

#include <chrono>

using namespace vob::misph::literals;

namespace vob::aoest
{
	struct presentation_time_world_component
	{
		using clock = std::chrono::high_resolution_clock;
		using time_point = std::chrono::time_point<clock>;
		using duration = std::chrono::duration<float>;

		time_point m_frameStartTime;
		duration m_frameDuration{ 0.0f };
		misph::measure_time m_elapsedTime = 0.0_s;
	};
}
