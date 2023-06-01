#pragma once

#include <vob/aoe/input/bindings.h>
#include <vob/aoe/input/inputs.h>

#include <vob/aoe/spacetime/presentation_time_world_component.h>

#include <vob/aoe/ecs/world_data_provider.h>
#include <vob/aoe/ecs/world_component_ref.h>


namespace vob::aoein
{
	class binding_system
	{
	public:
		explicit binding_system(aoecs::world_data_provider& a_wdp)
			: m_bindings{ a_wdp }
			, m_inputs{ a_wdp }
			, m_presentationTime{ a_wdp }
		{}

		void update() const
		{
			m_bindings->update(*m_inputs, m_presentationTime->m_elapsedTime);
		}

	private:
		aoecs::world_component_ref<bindings> m_bindings;
		aoecs::world_component_ref<inputs> m_inputs;
		aoecs::world_component_ref<aoest::presentation_time_world_component const> m_presentationTime;
	};
}
