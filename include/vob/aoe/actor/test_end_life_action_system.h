#pragma once

#include <vob/aoe/actor/action_component.h>

#include <vob/aoe/common/space/Transformcomponent.h>
#include <vob/aoe/common/time/LifetimeComponent.h>

#include <vob/aoe/ecs/world_data_provider.h>

#include <vob/misc/physics/measure_literals.h>


namespace vob::aoeac
{
	using namespace misph::literals;

	struct test_end_life_action_system
	{
		using action_components = aoecs::ComponentTypeList<
			aoe::common::TransformComponent const, action_component>;
		using timed_components = aoecs::ComponentTypeList<
			aoe::common::TransformComponent const, aoe::common::LifetimeComponent>;

		explicit test_end_life_action_system(aoecs::world_data_provider& a_wdp)
			: m_actions{ a_wdp.get_old_entity_view_list(*this, action_components{}) }
			, m_timedEntities{ a_wdp.get_old_entity_view_list(*this, timed_components{}) }
		{}

		void update() const
		{
			for (auto const& actionEntity : m_actions)
			{
				auto& actionComponent =
					actionEntity.get_component<action_component>();
				if (actionComponent.m_canInteract && actionComponent.m_isInteracting)
				{
					auto& actionTransform =
						actionEntity.get_component<aoe::common::TransformComponent>();

					for (auto const& timedEntity : m_timedEntities)
					{
						auto const& timedTransform =
							timedEntity.get_component<aoe::common::TransformComponent>();
						auto const dist =
							aoe::common::distance(actionTransform, timedTransform);
						if (dist < 3.0f)
						{
							auto& lifetime =
								timedEntity.get_component<aoe::common::LifetimeComponent>();
							lifetime.m_remainingTime = 0.0_s;
							actionComponent.m_isInteracting = false;
							break;
						}
					}
				}
			}
		}

		_aoecs::entity_view_list<
			aoe::common::TransformComponent const
			, action_component
		> const& m_actions;
		_aoecs::entity_view_list<
			aoe::common::TransformComponent const
			, aoe::common::LifetimeComponent
		> const& m_timedEntities;
	};
}
