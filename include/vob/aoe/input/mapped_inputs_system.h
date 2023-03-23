#pragma once

#include <vob/aoe/input/mapped_inputs_world_component.h>
#include <vob/aoe/input/physical_inputs_world_component.h>

#include <vob/aoe/spacetime/presentation_time_world_component.h>

#include <vob/aoe/ecs/world_data_provider.h>
#include <vob/aoe/ecs/world_component_ref.h>


namespace vob::aoein
{
	class mapped_input_system
	{
	public:
		explicit mapped_input_system(aoecs::world_data_provider& a_wdp)
			: m_mappedInputsComponent{ a_wdp }
			, m_physicalInputsComponent{ a_wdp }
			, m_presentationTimeComponent{ a_wdp }
		{}

		void update() const
		{
			for (auto& mappedAxis : m_mappedInputsComponent->m_axes)
			{
				mappedAxis->update(
					m_physicalInputsComponent->m_inputs,
					m_presentationTimeComponent->m_elapsedTime);
			}

			for (auto& mappedSwitch : m_mappedInputsComponent->m_switches)
			{
				mappedSwitch->update(
					m_physicalInputsComponent->m_inputs,
					m_presentationTimeComponent->m_elapsedTime);
			}
		}

	private:
		aoecs::world_component_ref<mapped_inputs_world_component> m_mappedInputsComponent;
		aoecs::world_component_ref<physical_inputs_world_component const>
			m_physicalInputsComponent;
		aoecs::world_component_ref<aoest::presentation_time_world_component const>
			m_presentationTimeComponent;
	};
}