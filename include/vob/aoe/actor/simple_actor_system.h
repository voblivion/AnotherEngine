#pragma once

#include <vob/aoe/actor/actor_component.h>
#include <vob/aoe/actor/action_component.h>

#include <vob/aoe/spacetime/transform.h>

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
				auto [position, rotation, interactorComponent] = interactorsView.get(interactorEntity);
				
				for (auto const& actionId : interactorComponent.m_actions)
				{
					auto const actionEntityIt = actionView.find(actionId);
					if (actionEntityIt != actionView.end())
					{
						auto [interactionPosition, interactionRotation] = actionView.get(*actionEntityIt);

						interactionPosition = position;
						interactionRotation = rotation;
					}
				}
			}
		}

	private:
		aoeng::registry_view_ref<
			aoest::position const,
			aoest::rotation const,
			actor_component const
		> m_interactors;
		aoeng::registry_view_ref<aoest::position, aoest::rotation> m_actions;
	};
}
