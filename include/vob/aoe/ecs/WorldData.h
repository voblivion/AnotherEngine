#pragma once

#include <vob/aoe/ecs/EntityManager.h>
#include <vob/aoe/ecs/ComponentManager.h>

namespace vob::aoecs
{
	struct WorldData
	{
		// Constructors
		explicit VOB_AOE_API WorldData(ComponentManager a_worldComponents);

		// Methods
		void update();

		// Attributes
		bool m_shouldStop = false;
		ComponentManager m_worldComponents;
		EntityManager m_entityManager;
	};
}
