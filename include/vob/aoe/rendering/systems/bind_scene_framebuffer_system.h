#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/world_components/scene_texture_world_component.h>

#include <vob/aoe/engine/world_data_provider.h>


namespace vob::aoegl
{
	class VOB_AOE_API bind_scene_framebuffer_system
	{
	public:
		explicit bind_scene_framebuffer_system(aoeng::world_data_provider& a_wdp);

		void update() const;

	private:
		aoeng::world_component_ref<scene_texture_world_component> m_sceneTextureWorldComponent;
	};
}