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
		public:
			// Constructors
			explicit WorldData(sta::Allocator<std::byte> const& a_allocator)
				: m_worldComponents{ a_allocator }
				, m_entityManager{ a_allocator }
			{}

			// Attributes
			bool m_shouldStop = false;
			ComponentManager m_worldComponents;
			EntityManager m_entityManager;
		};
	}
}
