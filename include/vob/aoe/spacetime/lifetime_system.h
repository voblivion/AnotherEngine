#pragma once

#include <vob/aoe/spacetime/lifetime_component.h>
#include <vob/aoe/spacetime/time_world_component.h>

#include <vob/aoe/engine/world_data_provider.h>


namespace vob::aoest
{
	class lifetime_system
	{
	public:
		explicit lifetime_system(aoeng::world_data_provider& a_wdp)
			: m_simulationTimeWorldComponent{ a_wdp }
			, m_lifetimeEntities{ a_wdp }
			, m_queryRef{ a_wdp }
		{}

		void update() const
		{
			auto elapsedTime = m_simulationTimeWorldComponent->m_elapsedTime;

			auto lifetimeEntities = m_lifetimeEntities.get();
			for (auto lifetimeEntity : lifetimeEntities)
			{
				auto& lifetime = lifetimeEntities.get<lifetime_component>(lifetimeEntity);
				lifetime.m_remainingLifetime -= elapsedTime;
				if (lifetime.m_remainingLifetime < 0.0_s)
				{
					m_queryRef.add(
						[lifetimeEntity](aoeng::entity_registry& a_registry) {
							a_registry.destroy(lifetimeEntity);
						});
				}
			}
		}

	private:
		aoeng::world_component_ref<simulation_time_world_component> m_simulationTimeWorldComponent;

		aoeng::registry_view_ref<lifetime_component> m_lifetimeEntities;

		aoeng::pending_entity_registry_query_queue_ref m_queryRef;
	};
}
