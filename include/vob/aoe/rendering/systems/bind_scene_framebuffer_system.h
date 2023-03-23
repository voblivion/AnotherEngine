#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/world_components/scene_texture_world_component.h>

#include <vob/aoe/ecs/world_component_ref.h>
#include <vob/aoe/ecs/world_data_provider.h>


namespace vob::aoegl
{
	class VOB_AOE_API bind_scene_framebuffer_system
	{
	public:
		explicit bind_scene_framebuffer_system(aoecs::world_data_provider& a_wdp);

		void update() const;

	private:
		aoecs::world_component_ref<scene_texture_world_component> m_sceneTextureWorldComponent;
	};
}