#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/engine/world_data_provider.h>
#include <vob/aoe/physics/collider_component.h>
#include <vob/aoe/physics/physics_world_component.h>
#include <vob/aoe/rendering/world_components/debug_mesh_world_component.h>


#include <vob/aoe/spacetime/transform_component.h>
#include <vob/aoe/spacetime/time_world_component.h>

#include <functional>


namespace vob::aoeph
{
	class VOB_AOE_API physics_system
	{
	public:
		explicit physics_system(aoeng::world_data_provider& a_wdp);

		void update() const;

	private:
		struct rigidbody_component
		{
			std::shared_ptr<btRigidBody> m_instance;
		};

		struct motion_state_component
		{
			std::shared_ptr<btMotionState> m_instance;
		};

		aoeng::world_component_ref<physics_world_component> m_physicsWorldComponent;
		aoeng::world_component_ref<
			aoest::simulation_time_world_component> m_simulationTimeWorldComponent;
		aoeng::world_component_ref<
			aoegl::debug_mesh_world_component> m_debugMeshWorldComponent;
		aoeng::world_component_ref<
			aoein::bindings const> m_bindings;

		aoeng::registry_view_ref<
			aoest::transform_component, collider_component, motion_state_component> m_colliderEntities;

		void step_physic_world() const;

		void update_transforms() const;

		void update_transform(
			collider_component const& a_collider,
			motion_state_component const& a_motionState,
			aoest::transform_component& a_transform) const;

		void try_debug_draw() const;

		void on_spawn(aoeng::entity_registry& a_registry, aoeng::entity a_entity) const;
		void on_despawn(aoeng::entity_registry& a_registry, aoeng::entity a_entity) const;

		motion_state_component& create_motion_state(aoeng::entity_handle a_entity,
			aoest::transform_component const& a_transform,
			collider_component const& a_collider) const;

		rigidbody_component& create_rigidbody(
			aoeng::entity_handle a_entity,
			aoeph::collider_component const& a_collider,
			motion_state_component& a_motionState) const;
	};
}
