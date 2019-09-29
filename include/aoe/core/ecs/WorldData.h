#pragma once

#include <aoe/core/ecs/EntityManager.h>
#include <aoe/core/ecs/ComponentManager.h>

namespace aoe
{
	namespace ecs
	{
		struct WorldData
		{
			// Constructors
			explicit AOE_CORE_API WorldData(
				ComponentManager a_worldComponents);

			// Methods
			void update();

			AOE_CORE_API sta::Allocator<std::byte> getAllocator() const;

			// Attributes
			bool m_shouldStop = false;
			ComponentManager m_worldComponents;
			EntityManager m_entityManager;
		};
	}
}
