#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/components/camera_component.h>
#include <vob/aoe/rendering/world_components/debug_render_world_component.h>
#include <vob/aoe/rendering/world_components/debug_mesh_world_component.h>
#include <vob/aoe/rendering/world_components/director_world_component.h>

#include <vob/aoe/ecs/entity_map_observer_list_ref.h>
#include <vob/aoe/ecs/world_component_ref.h>
#include <vob/aoe/ecs/world_data_provider.h>
#include <vob/aoe/spacetime/transform_component.h>
#include <vob/aoe/window/window_world_component.h>


namespace vob::aoegl
{
	class VOB_AOE_API render_debug_mesh_system
	{
	public:
		explicit render_debug_mesh_system(aoeng::world_data_provider& a_wdp);

		void update() const;

	private:
		aoeng::registry_view_ref<
			aoest::transform_component const, camera_component const> m_cameraEntities;

		aoeng::world_component_ref<aoewi::window_world_component> m_windowWorldComponent;
		aoeng::world_component_ref<director_world_component> m_directorWorldComponent;
		aoeng::world_component_ref<debug_render_world_component> m_debugRenderWorldComponent;
		aoeng::world_component_ref<debug_mesh_world_component> m_debugSceneWorldComponent;
	};
}
