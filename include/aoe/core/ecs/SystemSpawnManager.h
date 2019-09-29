#pragma once

#include <mutex>
#include <vector>

#include <aoe/core/Export.h>
#include <aoe/core/ecs/Entity.h>

namespace aoe
{
	namespace ecs
	{
		class SystemSpawnManager
		{
		public:
			// Constructors
			AOE_CORE_API explicit SystemSpawnManager(
				std::pmr::vector<sta::PolymorphicPtr<Entity>>& a_frameSpawns);

			// Methods
			AOE_CORE_API Entity& spawn(ComponentManager a_componentManager);

		private:
			// Attributes
			EntityId m_nextEntityId{};
			std::mutex m_mutex;
			std::pmr::vector<sta::PolymorphicPtr<Entity>>& m_frameSpawns;
		};
	}
}