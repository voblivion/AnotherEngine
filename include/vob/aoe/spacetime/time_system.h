#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/spacetime/time_world_component.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include <vob/aoe/engine/world_data_provider.h>


namespace vob::aoest
{
	class VOB_AOE_API time_system
	{
	public:
		explicit time_system(aoeng::world_data_provider& a_wdp)
			: m_presentationTimeContext{ a_wdp }
			, m_simulationTimeContext{ a_wdp }
		{}

		void update() const;

	private:
		aoeng::world_component_ref<presentation_time_context> m_presentationTimeContext;
		aoeng::world_component_ref<simulation_time_context> m_simulationTimeContext;
	};
}
