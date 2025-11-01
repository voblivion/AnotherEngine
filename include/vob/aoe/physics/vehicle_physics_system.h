#pragma once

#include <vob/aoe/api.h>
#include <vob/aoe/engine/world_data_provider.h>

#include <vob/aoe/physics/physics_context.h>
#include <vob/aoe/physics/vehicle_physics_component.h>
#include <vob/aoe/physics/components/rigidbody.h>
#include <vob/aoe/rendering/world_components/debug_mesh_world_component.h>
#include <vob/aoe/spacetime/time_world_component.h>
#include <vob/aoe/spacetime/transform.h>


namespace vob::aoeph
{
	class VOB_AOE_API vehicle_physics_system
	{
	public:
		explicit vehicle_physics_system(aoeng::world_data_provider& a_wdp);

		void update() const;

	private:
		aoeng::world_component_ref<aoest::simulation_time_context> m_simulationTimeContext;
		aoeng::world_component_ref<physics_context> m_physicsContext;
		aoeng::world_component_ref<aoegl::debug_mesh_world_component> m_debugMeshContext;

		aoeng::registry_view_ref<aoest::position, aoest::rotation, static_collider> m_blockerEntities;
		aoeng::registry_view_ref<aoest::position, aoest::rotation, linear_velocity, angular_velocity_local, car_collider> m_carColliderEntities;

		// TODO: debug
		aoeng::world_component_ref<aoein::inputs const> m_inputs;
	};
}
