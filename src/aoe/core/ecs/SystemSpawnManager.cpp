#include <vob/aoe/core/ecs/SystemSpawnManager.h>

namespace vob::aoe::ecs
{
	// Public
	SystemSpawnManager::SystemSpawnManager(std::vector<std::unique_ptr<Entity>>& a_frameSpawns)
		: m_frameSpawns{ a_frameSpawns } 
	{}

	Entity& SystemSpawnManager::spawn(ComponentManager a_componentManager)
	{
		std::lock_guard<std::mutex> t_lock{ m_mutex };
		auto& t_entity = m_frameSpawns.emplace_back(
			std::make_unique<Entity>(m_nextEntityId++, a_componentManager)
		);

		return *t_entity;
	}
}