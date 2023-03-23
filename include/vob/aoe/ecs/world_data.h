#pragma once

#include <vob/aoe/api.h>
#include <vob/aoe/ecs/_entity_manager.h>
#include <vob/aoe/ecs/component_set.h>
#include <vob/aoe/ecs/entity_manager.h>
#include <vob/aoe/ecs/stop_manager.h>

namespace vob::aoecs
{
	struct world_data
	{
		// Constructors
		explicit VOB_AOE_API world_data(
			component_set a_worldComponents
			, component_list_factory const& a_componentListFactory);

		// Methods
		void update();

		// Attributes
		stop_manager m_stopManager;
		component_set m_worldComponents;
		_aoecs::entity_manager m_oldEntityManager;
		entity_manager m_entityManager;
	};
}
