#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/world_components/post_process_render_world_component.h>
#include <vob/aoe/rendering/world_components/scene_texture_world_component.h>

#include <vob/aoe/window/window_world_component.h>

#include <vob/aoe/ecs/world_component_ref.h>
#include <vob/aoe/ecs/world_data_provider.h>


namespace vob::aoegl
{
	class VOB_AOE_API render_scene_system
	{
	public:
		explicit render_scene_system(aoecs::world_data_provider& a_wdp);

		void update() const;

	private:
		aoecs::world_component_ref<post_process_render_world_component> m_postProcessRenderWorldComponent;
		aoecs::world_component_ref<scene_texture_world_component const> m_sceneTextureWorldComponent;
		aoecs::world_component_ref<aoewi::window_world_component const> m_windowWorldComponent;
	};
}
