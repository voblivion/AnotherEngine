#pragma once

#include <vob/aoe/api.h>
#include <vob/aoe/common/time/Lifetimecomponent.h>
#include <vob/aoe/common/time/WorldTimecomponent.h>
#include <vob/aoe/ecs/WorldDataProvider.h>


namespace vob::aoe::common
{
	class LifetimeSystem final
	{
	public:
		using Components = aoecs::ComponentTypeList<LifetimeComponent>;

		// Constructors
		explicit LifetimeSystem(aoecs::WorldDataProvider& a_wdp)
			: m_unspawnManager{ a_wdp.get_unspawn_manager() }
			, m_worldTimeComponent{ *a_wdp.getWorldComponent<WorldTimeComponent const>() }
			, m_entities{ a_wdp.getentity_view_list(*this, Components{}) }
		{}

		// Methods
		void update() const
		{
			for(auto const& t_entity : m_entities)
			{
				auto& t_lifetime = t_entity.get_component<LifetimeComponent>();
				t_lifetime.m_remainingTime -= m_worldTimeComponent.m_elapsedTime;
				if (t_lifetime.m_remainingTime <= misph::measure_time{ 0.0f })
				{
					m_unspawnManager.unspawn(t_entity.get_id());
				}
			}
		}

	private:
		// Attributes
		aoecs::unspawn_manager& m_unspawnManager;
		WorldTimeComponent const& m_worldTimeComponent;
		aoecs::entity_view_list<LifetimeComponent> const& m_entities;
	};
}
