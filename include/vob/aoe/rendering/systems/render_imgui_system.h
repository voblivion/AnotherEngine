#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/engine/world_data_provider.h>

namespace vob::aoegl
{
	struct imgui_context_world_component;
}

namespace vob::aoewi
{
	struct window_input_world_component;
	struct window_world_component;
}


namespace vob::aoegl
{
	class VOB_AOE_API render_imgui_system
	{
	public:
		explicit render_imgui_system(aoeng::world_data_provider& a_wdp);

		~render_imgui_system();

		void update() const;

	private:
		aoeng::world_component_ref<aoewi::window_input_world_component> m_windowInputWorldComponent;
		aoeng::world_component_ref<aoewi::window_world_component> m_windowWorldComponent;
		aoeng::world_component_ref<aoegl::imgui_context_world_component> m_imGuiContextWorldComponent;
	};
}