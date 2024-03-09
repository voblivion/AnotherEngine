#pragma once

#include <vob/aoe/spacetime/time.h>


namespace vob::aoest
{
	enum class timer_state
	{
		play,
		pause,
		step
	};

	template <typename TCategory>
	struct pause_world_component
	{
		timer_state m_state = timer_state::play;
	};

	using simulation_pause_world_component = pause_world_component<simulation_time_t>;
}
