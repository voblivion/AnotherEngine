#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/engine/world_data_provider.h>

#include <vob/aoe/window/window_world_component.h>


namespace vob::aoegl
{
	class VOB_AOE_API bind_window_framebuffer_system
	{
	public:
		explicit bind_window_framebuffer_system(aoeng::world_data_provider& a_wdp);

		void update() const;

	private:
		aoeng::world_component_ref<aoewi::window_world_component> m_windowWorldComponent;
	};
}
