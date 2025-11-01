#pragma once

#include <vob/misc/physics/measure.h>
#include <vob/misc/physics/measure_literals.h>

#include <chrono>


namespace vob::aoeph
{
	using namespace std::chrono;
	using namespace misph::literals;

	struct physics_context
	{
		std::chrono::high_resolution_clock::time_point m_lastUpdateTime = {};
		std::chrono::high_resolution_clock::duration updateDuration = 10ms;
		int32_t updateStepCount = 10;
		bool just_updated = false;
	};
}
