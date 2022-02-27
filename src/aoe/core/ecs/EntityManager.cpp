#include <vob/aoe/ecs/EntityManager.h>


namespace vob::aoecs
{
	// Public
	EntityManager::EntityManager()
		: m_systemSpawnManager{ m_frameSpawns }
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


	SystemSpawnManager& EntityManager::getSystemSpawnManager()
	{
		return m_systemSpawnManager;
	}

	SystemUnspawnManager& EntityManager::getSystemUnspawnManager()
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
		for (auto const t_id : m_frameUnspawns)
		{
			auto const t_it = m_entities.find(t_id);
			if (t_it != m_entities.end())
			{
				for (auto& t_listHolder : m_systemEntityLists)
				{
					t_listHolder->onEntityRemoved(*t_it->second);
				}
				m_entities.erase(t_it);
			}
		}
		m_frameUnspawns.clear();
	}
}