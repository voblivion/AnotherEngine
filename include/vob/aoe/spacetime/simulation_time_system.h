#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/spacetime/simulation_time_world_component.h>

#include <vob/aoe/ecs/world_data_provider.h>
#include <vob/aoe/ecs/world_component_ref.h>


namespace vob::aoest
{
	class simulation_time_system
	{
	public:
		explicit simulation_time_system(aoecs::world_data_provider& a_wdp)
			: m_simulationTimeWorldComponent{ a_wdp }
		{}

		void update() const
		{
			auto const currentTime = simulation_time_world_component::clock::now();
			if (m_simulationTimeWorldComponent->m_frameStartTime
				!= simulation_time_world_component::time_point{})
			{
				m_simulationTimeWorldComponent->m_frameDuration =
					currentTime - m_simulationTimeWorldComponent->m_frameStartTime;
				m_simulationTimeWorldComponent->m_elapsedTime =
					m_simulationTimeWorldComponent->m_frameDuration;
			}
			m_simulationTimeWorldComponent->m_frameStartTime = currentTime;
		}

	private:
		// TODO pause?
		aoecs::world_component_ref<simulation_time_world_component> m_simulationTimeWorldComponent;
	};
}
