#pragma once

#include <vob/aoe/api.h>
#include <vob/aoe/common/time/Lifetimecomponent.h>
#include <vob/aoe/common/time/WorldTimecomponent.h>
#include <vob/aoe/ecs/world_data_provider.h>
#include <vob/aoe/ecs/entity_map_observer_list_ref.h>


namespace vob::aoe::common
{
	class LifetimeSystem final
	{
	public:
		using Components = aoecs::ComponentTypeList<LifetimeComponent>;

		// Constructors
		explicit LifetimeSystem(aoecs::world_data_provider& a_wdp)
			: m_unspawnManager{ a_wdp.get_despawner() }
			, m_worldTimeComponent{ a_wdp.get_world_component<WorldTimeComponent const>() }
			, m_entities{ a_wdp }
		{}

		// Methods
		void update() const
		{
			for(auto const& t_entity : m_entities)
			{
				auto& t_lifetime = t_entity.get<LifetimeComponent>();
				t_lifetime.m_remainingTime -= m_worldTimeComponent.m_elapsedTime;
				if (t_lifetime.m_remainingTime <= misph::measure_time{ 0.0f })
				{
					m_unspawnManager.despawn(t_entity.get_id());
				}
			}
		}

	private:
		// Attributes
		aoecs::entity_manager::despawner& m_unspawnManager;
		WorldTimeComponent const& m_worldTimeComponent;
		aoecs::entity_map_observer_list_ref<LifetimeComponent> m_entities;
	};
}
