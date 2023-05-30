#pragma once

#include <vob/aoe/actor/actor_component.h>
#include <vob/aoe/actor/action_component.h>

#include <vob/aoe/common/space/Transformcomponent.h>

#include <vob/aoe/ecs/world_data_provider.h>
#include <vob/aoe/ecs/entity_map_observer_list_ref.h>


namespace vob::aoeac
{
	struct simple_actor_system
	{
		explicit simple_actor_system(aoecs::world_data_provider& a_wdp)
			: m_interactors{ a_wdp }
			, m_actions{ a_wdp }
		{}

		void update() const
		{
			for (auto const& interactorEntity : m_interactors)
			{
				auto const& interactorComponent =
					interactorEntity.get<actor_component>();
				auto const& interactorTransform =
					interactorEntity.get<aoe::common::TransformComponent>();
				
				for (auto const& actionId : interactorComponent.m_actions)
				{
					auto const actionEntity = m_actions.find(actionId);
					if (actionEntity != m_actions.end())
					{
						auto& interactionTransform =
							actionEntity->get<aoe::common::TransformComponent>();

						interactionTransform.m_matrix = interactorTransform.m_matrix;
					}
				}
			}
		}

	private:
		aoecs::entity_map_observer_list_ref<
			aoe::common::TransformComponent const,
			actor_component const
		> m_interactors;
		aoecs::entity_map_observer_list_ref<aoe::common::TransformComponent> m_actions;
	};
}
