#pragma once

#include <vob/aoe/physics/rigidbody_component.h>
#include <vob/aoe/physics/world_physic_component.h>

#include <vob/aoe/ecs/world_data_provider.h>
#include <vob/aoe/spacetime/transform_component.h>
#include <vob/aoe/spacetime/simulation_time_world_component.h>

#include <functional>


namespace vob::aoeph
{
	class physic_system
	{
	public:
		explicit physic_system(aoecs::world_data_provider& a_wdp)
			: m_worldPhysicComponent{ a_wdp.get_world_component<world_physic_component>() }
			, m_worldSimulationTimeComponent{
				a_wdp.get_world_component<aoest::simulation_time_world_component>() }
			, m_rigidbodyEnties{
				a_wdp.observe_entities<aoest::transform_component, rigidbody_component>() }
		{
		}

		void update() const
		{
			auto& physicWorld = m_worldPhysicComponent.m_world.get();

			physicWorld.stepSimulation(m_worldSimulationTimeComponent.m_elapsedTime.get_value());

			for (auto& rigidbodyEntity : m_rigidbodyEnties)
			{
				auto& transform = rigidbodyEntity.get<aoest::transform_component>();
				auto& rigidbody = rigidbodyEntity.get<rigidbody_component>();
			}
		}

	private:
		world_physic_component& m_worldPhysicComponent;
		aoest::simulation_time_world_component& m_worldSimulationTimeComponent;

		aoecs::entity_map_observer_list<
			aoest::transform_component, rigidbody_component> const& m_rigidbodyEnties;
	};
}
