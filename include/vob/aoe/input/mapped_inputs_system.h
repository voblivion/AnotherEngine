#pragma once

#include <vob/aoe/input/mapped_inputs_world_component.h>
#include <vob/aoe/input/physical_inputs_world_component.h>

#include <vob/aoe/common/time/WorldTimeComponent.h>

#include <vob/aoe/ecs/WorldDataProvider.h>


namespace vob::aoein
{
	class mapped_input_system
	{
	public:
		explicit mapped_input_system(aoecs::WorldDataProvider& a_worldDataProvider)
			: m_mappedInputsComponent{ *a_worldDataProvider.getWorldComponent<mapped_inputs_world_component>() }
			, m_physicalInputsComponent{ *a_worldDataProvider.getWorldComponent<physical_inputs_world_component>() }
			, m_timeWorldComponent{ *a_worldDataProvider.getWorldComponent<aoe::common::WorldTimeComponent>() }
		{}

		void update() const
		{
			for (auto& mappedAxis : m_mappedInputsComponent.m_axes)
			{
				mappedAxis->update(m_physicalInputsComponent.m_inputs, m_timeWorldComponent.m_elapsedTime);
			}

			for (auto& mappedSwitch : m_mappedInputsComponent.m_switches)
			{
				mappedSwitch->update(m_physicalInputsComponent.m_inputs, m_timeWorldComponent.m_elapsedTime);
			}
		}

	private:
		mapped_inputs_world_component& m_mappedInputsComponent;
		physical_inputs_world_component const& m_physicalInputsComponent;
		aoe::common::WorldTimeComponent const& m_timeWorldComponent;
	};
}