#pragma once

#include <vob/aoe/api.h>
#include <vob/aoe/common/time/LifetimeComponent.h>
#include <vob/aoe/core/ecs/WorldDataProvider.h>


namespace vob::aoe::common
{
	class LifetimeSystem final
	{
	public:
		using Components = ecs::ComponentTypeList<LifetimeComponent>;

		// Constructors
		explicit LifetimeSystem(ecs::WorldDataProvider& a_wdp)
			: m_unspawnManager{ a_wdp.getUnspawnManager() }
			, m_worldTimeComponent{ *a_wdp.getWorldComponent<WorldTimeComponent const>() }
			, m_entities{ a_wdp.getEntityViewList(*this, Components{}) }
		{}

		// Methods
		void update() const
		{
			for(auto const& t_entity : m_entities)
			{
				auto& t_lifetime = t_entity.getComponent<LifetimeComponent>();
				t_lifetime.m_remainingTime -= m_worldTimeComponent.m_elapsedTime.value;
				if(t_lifetime.m_remainingTime <= 0)
				{
					m_unspawnManager.unspawn(t_entity.getId());
				}
			}
		}

	private:
		// Attributes
		ecs::SystemUnspawnManager& m_unspawnManager;
		WorldTimeComponent const& m_worldTimeComponent;
		ecs::EntityViewList<LifetimeComponent> const& m_entities;
	};
}
