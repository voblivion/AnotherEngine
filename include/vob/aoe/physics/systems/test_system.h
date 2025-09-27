#pragma once

// vob/aoe/physics/

// vob/aoe/
#include <vob/aoe/api.h>
#include <vob/aoe/engine/world_data_provider.h>
#include <vob/aoe/input/bindings.h>
#include <vob/aoe/rendering/components/camera_component.h>
#include <vob/aoe/rendering/world_components/debug_mesh_world_component.h>
#include <vob/aoe/rendering/world_components/director_world_component.h>
#include <vob/aoe/spacetime/time_world_component.h>
#include <vob/aoe/spacetime/transform.h>
#include <vob/aoe/window/window_world_component.h>
#include <vob/aoe/physics/components/rigidbody.h>

// vob/

// /


namespace vob::aoeph
{
	class VOB_AOE_API test_system
	{
	public:
		explicit test_system(aoeng::world_data_provider& a_wdp);

		void update() const;

	private:
		aoeng::world_component_ref<aoein::inputs const> m_inputs;
		aoeng::world_component_ref<aoein::bindings const> m_bindings;
		aoeng::world_component_ref<aoegl::debug_mesh_world_component> m_debugMeshWorldComponent;
		aoeng::world_component_ref<aoegl::director_world_component> m_directorWorldComponent;
		aoeng::world_component_ref<aoewi::window_world_component> m_windowWorldComponent;
		aoeng::world_component_ref<aoest::simulation_time_context> m_simulationTimeContext;

		aoeng::registry_view_ref<aoest::position const, aoest::rotation const, aoegl::camera_component const> m_cameraEntities;
		aoeng::registry_view_ref<aoest::position, aoest::rotation, linear_velocity, angular_velocity_local, dynamic_body> m_dynamicBodyEntities;
	};
}
