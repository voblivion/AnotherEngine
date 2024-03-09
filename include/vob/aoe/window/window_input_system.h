#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/input/inputs.h>
#include <vob/aoe/engine/world_data_provider.h>
#include <vob/aoe/window/window_input_world_component.h>
#include <vob/aoe/window/window_world_component.h>


namespace vob::aoewi
{
	class VOB_AOE_API window_input_system
	{
	public:
		explicit window_input_system(aoeng::world_data_provider& a_wdp);

		void update() const;

	private:
		aoeng::world_component_ref<window_world_component> m_windowWorldComponent;
		aoeng::world_component_ref<window_input_world_component const> m_windowInputWorldComponent;
		aoeng::world_component_ref<aoein::inputs> m_inputs;
		aoeng::should_stop_ref m_shouldStop;
	};
}
