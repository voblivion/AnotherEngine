#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/physics/rigidbody_component.h>
#include <vob/aoe/physics/world_physic_component.h>

#include <vob/aoe/ecs/world_data_provider.h>

#include <vob/aoe/engine/world_data_provider.h>

#include <vob/aoe/spacetime/transform_component.h>
#include <vob/aoe/spacetime/time_world_component.h>

#include <functional>


namespace vob::aoeph
{
	class VOB_AOE_API physic_system
	{
	public:
		explicit physic_system(aoeng::world_data_provider& a_wdp);

		void update() const;

	private:
		aoeng::world_component_ref<world_physic_component> m_physicsWorldComponent;
		aoeng::world_component_ref<
			aoest::simulation_time_world_component> m_simulationTimeWorldComponent;

		aoeng::registry_view_ref<aoest::transform_component, rigidbody_component> m_rigidbodyEntities;
	};
}
