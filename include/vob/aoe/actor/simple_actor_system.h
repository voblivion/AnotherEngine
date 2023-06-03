#pragma once

#include <vob/aoe/actor/actor_component.h>
#include <vob/aoe/actor/action_component.h>

#include <vob/aoe/spacetime/transform_component.h>

#include <vob/aoe/engine/world_data_provider.h>


namespace vob::aoeac
{
	struct simple_actor_system
	{
		explicit simple_actor_system(aoeng::world_data_provider& a_wdp)
			: m_interactors{ a_wdp }
			, m_actions{ a_wdp }
		{}

		void update() const
		{
			auto actionView = *m_actions;
			auto interactorsView = *m_interactors;
			for (auto const& interactorEntity : *m_interactors)
			{
				auto [interactorTransform, interactorComponent] = interactorsView.get(interactorEntity);
				
				for (auto const& actionId : interactorComponent.m_actions)
				{
					auto const actionEntityIt = actionView.find(actionId);
					if (actionEntityIt != actionView.end())
					{
						auto [interactionTransform] = actionView.get(*actionEntityIt);

						interactionTransform.m_matrix = interactorTransform.m_matrix;
					}
				}
			}
		}

	private:
		aoeng::registry_view_ref<
			aoest::transform_component const,
			actor_component const
		> m_interactors;
		aoeng::registry_view_ref<aoest::transform_component> m_actions;
	};
}
