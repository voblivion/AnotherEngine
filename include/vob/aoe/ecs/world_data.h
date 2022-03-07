#pragma once

#include <vob/aoe/api.h>
#include <vob/aoe/ecs/entity_manager.h>
#include <vob/aoe/ecs/component_manager.h>
#include <vob/aoe/ecs/stop_manager.h>

namespace vob::aoecs
{
	struct world_data
	{
		// Constructors
		explicit VOB_AOE_API world_data(component_manager a_worldComponents);

		// Methods
		void update();

		// Attributes
		stop_manager m_stopManager;
		component_manager m_worldComponents;
		entity_manager m_entityManager;
	};
}
