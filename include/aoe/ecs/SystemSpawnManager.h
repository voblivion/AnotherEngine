#pragma once

#include <mutex>
#include <vector>

#include <aoe/Config.h>
#include <aoe/ecs/Entity.h>

namespace aoe
{
	namespace ecs
	{
		class SystemSpawnManager
		{
		public:
			// Constructors
			AOE_API explicit SystemSpawnManager(
				std::pmr::vector<sta::PolymorphicPtr<Entity>>& a_frameSpawns);

			// Methods
			AOE_API Entity& spawn(ComponentManager const& a_componentManager);

		private:
			// Attributes
			EntityId m_nextEntityId{};
			std::mutex m_mutex;
			std::pmr::vector<sta::PolymorphicPtr<Entity>>& m_frameSpawns;
		};
	}
}