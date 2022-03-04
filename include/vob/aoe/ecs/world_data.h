#pragma once

#include <vob/aoe/ecs/EntityManager.h>
#include <vob/aoe/ecs/component_manager.h>

namespace vob::aoecs
{
	struct world_data
	{
		// Constructors
		explicit VOB_AOE_API world_data(component_manager a_worldComponents);

		// Methods
		void update();

		// Attributes
		bool m_shouldStop = false;
		component_manager m_worldComponents;
		EntityManager m_entityManager;
	};
}
