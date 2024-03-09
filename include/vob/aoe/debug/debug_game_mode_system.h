#pragma once

#include <vob/aoe/debug/controllable_tag.h>
#include <vob/aoe/debug/controlled_tag.h>
#include <vob/aoe/debug/debug_game_mode_world_component.h>
#include <vob/aoe/rendering/components/camera_component.h>
#include <vob/aoe/rendering/world_components/director_world_component.h>
#include <vob/aoe/spacetime/transform.h>


namespace vob::aoedb
{
	struct VOB_AOE_API debug_game_mode_system
	{
	public:
		explicit debug_game_mode_system(aoeng::world_data_provider& a_wdp);

		void update() const;

	private:
		aoeng::world_component_ref<aoein::bindings const> m_bindings;
		aoeng::world_component_ref<aoegl::director_world_component> m_directorWorldComponent;
		aoeng::world_component_ref<debug_game_mode_world_component> m_debugGameModeWorldComponent;
		aoeng::pending_entity_registry_query_queue_ref m_queryRef;

		aoeng::registry_view_ref<controlled_tag const> m_controlledEntities;
		aoeng::registry_view_ref<controllable_tag const> m_controllableEntities;
		aoeng::registry_view_ref<aoest::position, aoest::rotation, aoegl::camera_component> m_cameraEntities;
	};
}
