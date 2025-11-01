#pragma once

#include <vob/misc/physics/measure.h>
#include <vob/misc/physics/measure_literals.h>

#include <chrono>
#include <optional>


namespace vob::aoest
{
	using namespace misph::literals;

	struct presentation_time_context
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> tick_start_time = std::chrono::high_resolution_clock::now();
		misph::measure_time elapsed_time = 0.0_s;
	};

	struct simulation_time_context
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> tick_start_time = {};
		misph::measure_time elapsed_time = 0.0_s;
		misph::measure_time play_for_duration = 0.0_s;
	};
}
