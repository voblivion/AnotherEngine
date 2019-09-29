#include <aoe/core/ecs/SystemSpawnManager.h>

namespace aoe
{
	namespace ecs
	{
		// Public
		SystemSpawnManager::SystemSpawnManager(
			std::pmr::vector<sta::PolymorphicPtr<Entity>>& a_frameSpawns)
			: m_frameSpawns{ a_frameSpawns }
		{}

		Entity& SystemSpawnManager::spawn(ComponentManager a_componentManager)
		{
			auto const t_resource = m_frameSpawns.get_allocator().resource();
			std::lock_guard<std::mutex> t_lock{ m_mutex };
			auto& t_entity = m_frameSpawns.emplace_back(
				sta::allocatePolymorphicWith<Entity>(
					std::move(t_resource)
					, m_nextEntityId++
					, a_componentManager));

			return *t_entity;
		}
	}
}