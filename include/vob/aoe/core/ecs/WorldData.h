#pragma once

#include <vob/aoe/core/ecs/EntityManager.h>
#include <vob/aoe/core/ecs/ComponentManager.h>

namespace vob::aoe::ecs
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
