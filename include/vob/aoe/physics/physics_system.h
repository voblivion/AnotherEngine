#pragma once

#include <vob/aoe/api.h>
#include <vob/aoe/engine/world_data_provider.h>
#include <vob/aoe/physics/physics_context.h>
#include <vob/aoe/physics/physics_debug_context.h>
#include <vob/aoe/spacetime/time_world_component.h>

#include <functional>

// WIP vehicles / wheels
#include <vob/aoe/input/bindings.h>
#include <vob/aoe/spacetime/transform.h>
#include <vob/aoe/physics/components/rigidbody.h>
#include <vob/aoe/physics/components/car_controller.h>
#include <vob/aoe/rendering/world_components/debug_mesh_world_component.h>
#include <vob/aoe/rendering/world_components/imgui_context_world_component.h>


namespace vob::aoeph
{
	class VOB_AOE_API physics_debug_system
	{
	public:
		physics_debug_system(aoeng::world_data_provider& a_wdp);
		
		void update() const;

	private:
		aoeng::world_component_ref<physics_debug_context> m_physicsDebugContext;
		aoeng::world_component_ref<aoegl::debug_mesh_world_component> m_debugMeshContext;

		aoeng::registry_view_ref<aoest::position, aoest::rotation, linear_velocity, angular_velocity_local, dynamic_body> m_dynamicBodyEntities;
		aoeng::registry_view_ref<aoest::position, aoest::rotation, static_collider> m_staticColliderEntities;
		aoeng::registry_view_ref<aoest::position, aoest::rotation, linear_velocity, angular_velocity_local, car_collider> m_carColliderEntities;
	};

	class VOB_AOE_API physics_system
	{
	public:
		physics_system(aoeng::world_data_provider& a_wdp);

		void update() const;

	private:
		aoeng::world_component_ref<aoest::simulation_time_context> m_simulationTimeContext;
		aoeng::world_component_ref<physics_context> m_physicsContext;
		aoeng::world_component_ref<aoein::inputs const> m_inputs;

		aoeng::registry_view_ref<aoest::position, aoest::rotation, linear_velocity, angular_velocity_local, dynamic_body> m_dynamicBodyEntities;
		aoeng::registry_view_ref<aoest::position, aoest::rotation, static_collider> m_staticColliderEntities;
		aoeng::registry_view_ref<aoest::position, aoest::rotation, linear_velocity, angular_velocity_local, car_collider> m_carColliderEntities;

	};

}
