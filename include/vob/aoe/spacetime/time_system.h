#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/spacetime/time_world_component.h>

#include <vob/aoe/engine/world_data_provider.h>


namespace vob::aoest
{
	template <typename TCategory>
	class time_system
	{
	public:
		explicit time_system(aoeng::world_data_provider& a_wdp)
			: m_timeWorldComponent{ a_wdp }
		{}

		void update() const
		{
#pragma message(VOB_MISTD_TODO "handle pause.")
			auto const currentTime = time_world_component<TCategory>::clock::now();
			auto& tickStartTime = m_timeWorldComponent->m_tickStartTime;
			if (tickStartTime.has_value())
			{
				m_timeWorldComponent->m_elapsedTime = currentTime - *tickStartTime;
			}
			m_timeWorldComponent->m_tickStartTime = currentTime;
		}

	private:
		aoeng::world_component_ref<time_world_component<TCategory>> m_timeWorldComponent;
	};

	using presentation_time_system = time_system<presentation_time_t>;
	using simulation_time_system = time_system<simulation_time_t>;
}
