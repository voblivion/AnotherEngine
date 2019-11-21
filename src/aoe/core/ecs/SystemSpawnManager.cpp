#include <vob/aoe/core/ecs/SystemSpawnManager.h>

namespace vob::aoe::ecs
{
	// Public
	SystemSpawnManager::SystemSpawnManager(
		std::pmr::vector<sta::polymorphic_ptr<Entity>>& a_frameSpawns)
		: m_frameSpawns{ a_frameSpawns }
	{}

	Entity& SystemSpawnManager::spawn(ComponentManager a_componentManager)
	{
		auto const t_resource = m_frameSpawns.get_allocator().resource();
		std::lock_guard<std::mutex> t_lock{ m_mutex };
		auto& t_entity = m_frameSpawns.emplace_back(
			sta::allocate_polymorphic<Entity>(
				std::pmr::polymorphic_allocator<Entity>{ t_resource }
				, m_nextEntityId++
				, a_componentManager
			)
		);

		return *t_entity;
	}
}