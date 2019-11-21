#pragma once

#include <mutex>
#include <vector>

#include <vob/aoe/api.h>
#include <vob/aoe/core/ecs/Entity.h>

namespace vob::aoe::ecs
{
	class VOB_AOE_API SystemSpawnManager
	{
	public:
		// Constructors
		explicit SystemSpawnManager(
			std::pmr::vector<sta::polymorphic_ptr<Entity>>& a_frameSpawns);

		// Methods
		Entity& spawn(ComponentManager a_componentManager);

	private:
		// Attributes
		EntityId m_nextEntityId{};
		std::mutex m_mutex;
		std::pmr::vector<sta::polymorphic_ptr<Entity>>& m_frameSpawns;
	};
}