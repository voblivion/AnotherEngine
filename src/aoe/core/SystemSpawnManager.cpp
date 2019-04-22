#include <aoe/core/SystemSpawnManager.h>

namespace aoe
{
	namespace core
	{
		// Public
		SystemSpawnManager::SystemSpawnManager(
			std::pmr::vector<sta::PolymorphicPtr<Entity>>& a_frameSpawns)
			: m_frameSpawns{ a_frameSpawns }
		{}

		Entity& SystemSpawnManager::spawn(
			ComponentManager const& a_componentManager)
		{
			auto const t_allocator = m_frameSpawns.get_allocator();
			std::lock_guard<std::mutex> t_lock{m_mutex};
			m_frameSpawns.emplace_back(sta::allocatePolymorphic<Entity>(
				t_allocator
				, m_nextEntityId++, a_componentManager));
			return *(m_frameSpawns.back());
		}
	}
}