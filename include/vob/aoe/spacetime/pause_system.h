#pragma once

#include <vob/aoe/spacetime/pause_world_component.h>
#include <vob/aoe/spacetime/time_world_component.h>

#include <vob/aoe/engine/world_data_provider.h>


namespace vob::aoest
{
	template <typename TCategory>
	class pause_system
	{
	public:
		explicit pause_system(aoeng::world_data_provider& a_wdp)
			: m_timeWorldComponent{ a_wdp }
			, m_pauseWorldComponent{ a_wdp }
		{}

		void update() const
		{
			switch (m_pauseWorldComponent->m_state)
			{
			case timer_state::pause:
				m_timeWorldComponent->m_elapsedTime = 0.0_s;
				break;
			case timer_state::step:
				m_pauseWorldComponent->m_state = timer_state::pause;
				m_timeWorldComponent->m_elapsedTime = 1.0_s / 60;
				break;
			default:
				break;
			}
		}
		
	private:
		aoeng::world_component_ref<time_world_component<TCategory>> m_timeWorldComponent;
		aoeng::world_component_ref<pause_world_component<TCategory>> m_pauseWorldComponent;
	};

	using simulation_pause_system = pause_system<simulation_time_t>;
}
