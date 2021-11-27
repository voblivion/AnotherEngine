#pragma once

#include <mutex>
#include <vector>

#include <vob/aoe/api.h>
#include <vob/aoe/ecs/Entity.h>

namespace vob::aoecs
{
	class VOB_AOE_API SystemSpawnManager
	{
	public:
		// Constructors
		explicit SystemSpawnManager(std::vector<std::unique_ptr<Entity>>& a_frameSpawns);

		// Methods
		Entity& spawn(ComponentManager a_componentManager);

	private:
		// Attributes
		EntityId m_nextEntityId{};
		std::mutex m_mutex{};
		std::vector<std::unique_ptr<Entity>>& m_frameSpawns;
	};
}