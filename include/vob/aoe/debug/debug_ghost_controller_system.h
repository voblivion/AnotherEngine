#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/debug/controlled_tag.h>
#include <vob/aoe/debug/debug_ghost_controller_component.h>

#include <vob/aoe/engine/world_data_provider.h>
#include <vob/aoe/input/bindings.h>
#include <vob/aoe/spacetime/transform.h>
#include <vob/aoe/spacetime/time_world_component.h>


namespace vob::aoedb
{
	class VOB_AOE_API debug_ghost_controller_system
	{
	public:
		explicit debug_ghost_controller_system(aoeng::world_data_provider& a_wdp);

		void update() const;

	private:
		aoeng::world_component_ref<aoein::bindings const> m_bindings;
		aoeng::world_component_ref<
			aoest::presentation_time_world_component const> m_presentationTimeWorldComponent;
		aoeng::registry_view_ref<
			aoest::position, aoest::rotation, debug_ghost_controller_component, controlled_tag> m_ghostControllerEntities;
	};
}
