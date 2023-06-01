#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/input/inputs.h>

#include <vob/aoe/ecs/world_data_provider.h>
#include <vob/aoe/ecs/world_component_ref.h>

#include <vob/aoe/window/window_world_component.h>


namespace vob::aoewi
{
	class VOB_AOE_API window_input_system
	{
	public:
		explicit window_input_system(aoecs::world_data_provider& a_wdp);

		void update() const;

	private:
		aoecs::world_component_ref<window_world_component> m_windowWorldComponent;
		aoecs::world_component_ref<aoein::inputs> m_inputs;
		aoecs::stop_manager& m_stopManager;
	};
}
