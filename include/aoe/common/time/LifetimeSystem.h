#pragma once

#include <aoe/common/Export.h>
#include <aoe/common/time/LifetimeComponent.h>
#include <aoe/core/ecs/System.h>
#include <aoe/core/ecs/WorldDataProvider.h>


namespace aoe
{
	namespace common
	{
		class LifetimeSystem final
		{
		public:
			using Components = ecs::ComponentTypeList<LifetimeComponent>;

			// Constructors
			explicit LifetimeSystem(ecs::WorldDataProvider& a_wdp)
				: m_unspawnManager{ a_wdp.getUnspawnManager() }
				, m_worldTime{ *a_wdp.getWorldComponent<TimeComponent const>() }
				, m_entities{ a_wdp.getEntityList(*this, Components{}) }
			{}

			// Methods
			void update() const
			{
				for(auto const& t_entity : m_entities)
				{
					auto& t_lifetime = t_entity.getComponent<LifetimeComponent>();
					t_lifetime.m_remainingTime -= m_worldTime.m_elapsedTime;
					if(t_lifetime.m_remainingTime <= 0)
					{
						m_unspawnManager.unspawn(t_entity.getId());
					}
				}
			}

		private:
			// Attributes
			ecs::SystemUnspawnManager& m_unspawnManager;
			TimeComponent const& m_worldTime;
			ecs::SystemEntityList<LifetimeSystem, LifetimeComponent> const& m_entities;
		};
	}
}
