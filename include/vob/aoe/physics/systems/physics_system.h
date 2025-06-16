#pragma once

#include <vob/aoe/physics/world_components/physics_world_component.h>

#include <vob/aoe/api.h>
#include <vob/aoe/engine/world_data_provider.h>
#include <vob/aoe/physics/components/motion_state.h>
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
	class VOB_AOE_API physics_system
	{
	public:
		explicit physics_system(aoeng::world_data_provider& a_wdp);

		void update() const;

	private:
		aoeng::world_component_ref<physics_world_component> m_physicsWorldComponent;
		aoeng::world_component_ref<aoest::simulation_time_context> m_simulationTimeWorldComponent;
		aoeng::world_component_ref<aoegl::imgui_context_world_component> m_imGuiContextWorldComponent;
		aoeng::registry_view_ref<motion_state> m_movingEntities; // locking them

		// WIP vehicles / wheels
		aoeng::world_component_ref<aoein::inputs const> m_inputs;
		aoeng::world_component_ref<aoein::bindings const> m_bindings;
		aoeng::world_component_ref<aoegl::debug_mesh_world_component> m_debugMeshWorldComponent;
		aoeng::world_component_ref<car_controller_world_component> m_carControllerWorldComponent;
		aoeng::registry_view_ref<aoest::position, aoest::rotation, car_controller, rigidbody> m_carEntities;
		aoeng::registry_view_ref<aoest::position, aoest::rotation, car_controller> m_hingeCarEntities;
	};

	class VOB_AOE_API physx_debug_system
	{
	public:
		physx_debug_system(aoeng::world_data_provider& a_wdp);
		
		void update() const;

	private:
		aoeng::world_component_ref<physx_debug_context> m_physicsDebugContext;
		aoeng::world_component_ref<aoegl::debug_mesh_world_component> m_debugMeshContext;

		aoeng::registry_view_ref<aoest::position, aoest::rotation, linear_velocity, angular_velocity_local, dynamic_body> m_dynamicBodyEntities;
		aoeng::registry_view_ref<aoest::position, aoest::rotation, static_body> m_staticBodyEntities;
	};

	class VOB_AOE_API physx_system
	{
	public:
		physx_system(aoeng::world_data_provider& a_wdp);

		void update() const;

	private:
		aoeng::world_component_ref<aoest::simulation_time_context> m_simulationTimeContext;
		aoeng::world_component_ref<physx_context> m_physicsContext;
		aoeng::world_component_ref<aoein::inputs const> m_inputs;

		aoeng::registry_view_ref<aoest::position, aoest::rotation, linear_velocity, angular_velocity_local, dynamic_body> m_dynamicBodyEntities;
		aoeng::registry_view_ref<aoest::position, aoest::rotation, static_body> m_staticBodyEntities;
	};

}
