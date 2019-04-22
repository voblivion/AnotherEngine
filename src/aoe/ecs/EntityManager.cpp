#include <aoe/ecs/EntityManager.h>


namespace aoe
{
	namespace ecs
	{
		// Public
		EntityManager::EntityManager(
			sta::Allocator<std::byte> const& a_allocator)
			: m_systemEntityLists{ a_allocator }
			, m_frameSpawns{ a_allocator }
			, m_systemSpawnManager{ m_frameSpawns }
			, m_systemUnspawnManager{ m_frameUnspawns }
		{}

		SystemSpawnManager& EntityManager::getSystemSpawnManager()
		{
			return m_systemSpawnManager;
		}

		SystemUnspawnManager& EntityManager::getSystemUnspawnManager()
		{
			return m_systemUnspawnManager;
		}

		void EntityManager::processSpawns()
		{
			for (auto& t_entity : m_frameSpawns)
			{
				for (auto& t_listHolder : m_systemEntityLists)
				{
					t_listHolder->onEntityAdded(*t_entity);
				}
				m_entities.emplace(t_entity->getId(), std::move(t_entity));
			}
			m_frameSpawns.clear();
		}

		void EntityManager::processUnspawns()
		{
			for (auto const t_id : m_frameUnspawns)
			{
				for (auto& t_listHolder : m_systemEntityLists)
				{
					t_listHolder->onEntityRemoved(t_id);
				}
				auto const t_it = m_entities.find(t_id);
				m_entities.erase(t_it);
			}
			m_frameUnspawns.clear();
		}
	}
}