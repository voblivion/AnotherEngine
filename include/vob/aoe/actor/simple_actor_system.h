#pragma once

#include <vob/aoe/actor/actor_component.h>
#include <vob/aoe/actor/action_component.h>

#include <vob/aoe/common/space/Transformcomponent.h>

#include <vob/aoe/ecs/world_data_provider.h>


namespace vob::aoeac
{
	struct simple_actor_system
	{
		using interactor_components = aoecs::ComponentTypeList<
			aoe::common::TransformComponent const
			, actor_component const
		>;

		using action_components = aoecs::ComponentTypeList<aoe::common::TransformComponent>;

		explicit simple_actor_system(aoecs::world_data_provider& a_wdp)
			: m_interactors{ a_wdp.get_old_entity_view_list(*this, interactor_components{}) }
			, m_actions{ a_wdp.get_old_entity_view_list(*this, action_components{}) }
		{}

		void update() const
		{
			for (auto const& interactorEntity : m_interactors)
			{
				auto const& interactorComponent =
					interactorEntity.get_component<actor_component>();
				auto const& interactorTransform =
					interactorEntity.get_component<aoe::common::TransformComponent>();
				
				for (auto const& actionId : interactorComponent.m_actions)
				{
					auto const* actionEntity = m_actions.find(actionId);
					if (actionEntity != nullptr)
					{
						auto& interactionTransform =
							actionEntity->get_component<aoe::common::TransformComponent>();

						interactionTransform.m_matrix = interactorTransform.m_matrix;
					}
				}
			}
		}

	private:
		_aoecs::entity_view_list<
			aoe::common::TransformComponent const,
			actor_component const
		> const& m_interactors;
		_aoecs::entity_view_list<aoe::common::TransformComponent> const& m_actions;
	};
}
