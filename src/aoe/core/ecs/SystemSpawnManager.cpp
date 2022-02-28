#include <vob/aoe/ecs/SystemSpawnManager.h>

namespace vob::aoecs
{
	// Public
	SystemSpawnManager::SystemSpawnManager(std::vector<std::unique_ptr<entity>>& a_frameSpawns)
		: m_frameSpawns{ a_frameSpawns } 
	{}

	entity& SystemSpawnManager::spawn(component_manager a_componentManager)
	{
		std::lock_guard<std::mutex> t_lock{ m_mutex };
		auto& t_entity = m_frameSpawns.emplace_back(
			std::make_unique<entity>(entity_id{ m_nextEntityIdValue++ }, a_componentManager)
		);

		return *t_entity;
	}
}