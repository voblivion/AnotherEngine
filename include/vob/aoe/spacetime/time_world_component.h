#pragma once

#include <vob/misc/physics/measure.h>
#include <vob/misc/physics/measure_literals.h>

#include <chrono>
#include <optional>


namespace vob::aoest
{
	using namespace misph::literals;

	template <typename TCategory>
	struct time_world_component
	{
		using clock = std::chrono::high_resolution_clock;
		using time_point = std::chrono::time_point<clock>;

		std::optional<time_point> m_tickStartTime = std::nullopt;
		misph::measure_time m_elapsedTime = 0.0_s;
	};

	struct presentation_t {};
	struct simulation_t {};

	using presentation_time_world_component = time_world_component<presentation_t>;
	using simulation_time_world_component = time_world_component<simulation_t>;
}
