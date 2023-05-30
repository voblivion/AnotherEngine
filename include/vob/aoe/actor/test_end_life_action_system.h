#pragma once

#include <vob/aoe/actor/action_component.h>

#include <vob/aoe/common/space/Transformcomponent.h>
#include <vob/aoe/common/time/LifetimeComponent.h>

#include <vob/aoe/ecs/world_data_provider.h>
#include <vob/aoe/ecs/entity_map_observer_list_ref.h>

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
			: m_actions{ a_wdp }
			, m_timedEntities{ a_wdp }
		{}

		void update() const
		{
			for (auto const& actionEntity : m_actions)
			{
				auto& actionComponent =
					actionEntity.get<action_component>();
				if (actionComponent.m_canInteract && actionComponent.m_isInteracting)
				{
					auto& actionTransform =
						actionEntity.get<aoe::common::TransformComponent>();

					for (auto const& timedEntity : m_timedEntities)
					{
						auto const& timedTransform =
							timedEntity.get<aoe::common::TransformComponent>();
						auto const dist =
							aoe::common::distance(actionTransform, timedTransform);
						if (dist < 3.0f)
						{
							auto& lifetime =
								timedEntity.get<aoe::common::LifetimeComponent>();
							lifetime.m_remainingTime = 0.0_s;
							actionComponent.m_isInteracting = false;
							break;
						}
					}
				}
			}
		}

		aoecs::entity_map_observer_list_ref<
			aoe::common::TransformComponent const
			, action_component
		> m_actions;
		aoecs::entity_map_observer_list_ref<
			aoe::common::TransformComponent const
			, aoe::common::LifetimeComponent
		> m_timedEntities;
	};
}
