#include <vob/aoe/ecs/EntityManager.h>


namespace vob::aoecs
{
	// Public
	EntityManager::EntityManager()
		: m_systemSpawnManager{ m_frameSpawns, m_unusedEntityIds }
		, m_systemUnspawnManager{ m_frameUnspawns }
	{}

	EntityManager::~EntityManager()
	{
		for (auto& t_entity : m_entities)
		{
			for (auto& t_listHolder : m_systemEntityLists)
			{
				t_listHolder->onEntityRemoved(*t_entity.second);
			}
		}
	}


	system_spawn_manager& EntityManager::getSystemSpawnManager()
	{
		return m_systemSpawnManager;
	}

	system_unspawn_manager& EntityManager::getSystemUnspawnManager()
	{
		return m_systemUnspawnManager;
	}

	void EntityManager::update()
	{
		processUnspawns();

		processSpawns();
	}

	void EntityManager::processSpawns()
	{
		for (auto& t_entity : m_frameSpawns)
		{
			for (auto& t_listHolder : m_systemEntityLists)
			{
				t_listHolder->onEntityAdded(*t_entity);
			}
			m_entities.emplace(t_entity->get_id(), std::move(t_entity));
		}
		m_frameSpawns.clear();
	}

	void EntityManager::processUnspawns()
	{
		for (auto const id : m_frameUnspawns)
		{
			m_unusedEntityIds.emplace_back(id);
			auto const it = m_entities.find(id);
			if (it != m_entities.end())
			{
				for (auto& t_listHolder : m_systemEntityLists)
				{
					t_listHolder->onEntityRemoved(*it->second);
				}
				m_entities.erase(it);
			}
		}
		m_frameUnspawns.clear();
	}
}