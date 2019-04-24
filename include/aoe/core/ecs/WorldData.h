#pragma once

#include <aoe/core/ecs/EntityManager.h>
#include <aoe/core/ecs/ComponentManager.h>
#include <aoe/core/standard/Allocator.h>

namespace aoe
{
	namespace ecs
	{
		struct WorldData
		{
			// Constructors
			explicit AOE_CORE_API WorldData(
				sta::Allocator<std::byte> const& a_allocator);

			// Methods
			void update();

			// Attributes
			bool m_shouldStop = false;
			ComponentManager m_worldComponents;
			EntityManager m_entityManager;
		};
	}
}
