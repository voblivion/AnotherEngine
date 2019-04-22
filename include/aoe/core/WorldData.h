#pragma once

#include <aoe/core/EntityManager.h>
#include <aoe/core/ComponentManager.h>
#include <aoe/standard/Allocator.h>

namespace aoe
{
	namespace core
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
