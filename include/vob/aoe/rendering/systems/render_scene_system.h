#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/world_components/post_process_render_world_component.h>
#include <vob/aoe/rendering/world_components/scene_texture_world_component.h>

#include <vob/aoe/window/window_world_component.h>

#include <vob/aoe/ecs/world_component_ref.h>
#include <vob/aoe/ecs/world_data_provider.h>

#include <vob/aoe/engine/world_data_provider.h>


namespace vob::aoegl
{
	class VOB_AOE_API render_scene_system
	{
	public:
		explicit render_scene_system(aoeng::world_data_provider& a_wdp);

		void update() const;

	private:
		aoeng::world_component_ref<post_process_render_world_component> m_postProcessRenderWorldComponent;
		aoeng::world_component_ref<scene_texture_world_component const> m_sceneTextureWorldComponent;
		aoeng::world_component_ref<aoewi::window_world_component const> m_windowWorldComponent;
	};
}
