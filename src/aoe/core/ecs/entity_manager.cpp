#include <vob/aoe/ecs/entity_manager.h>


namespace vob::aoecs
{
	// Public
	entity_manager::entity_manager()
		: m_spawnManager{ m_frameSpawns, m_unusedEntityIds }
		, m_unspawnManager{ m_frameUnspawns }
	{}

	entity_manager::~entity_manager()
	{
		for (auto& t_entity : m_entities)
		{
			for (auto& t_listHolder : m_systemEntityViewLists)
			{
				t_listHolder->on_entity_removed(*t_entity.second);
			}
		}
	}


	spawn_manager& entity_manager::get_spawn_manager()
	{
		return m_spawnManager;
	}

	unspawn_manager& entity_manager::get_unspawn_manager()
	{
		return m_unspawnManager;
	}

	void entity_manager::update()
	{
		process_unspawns();

		process_spawns();
	}

	void entity_manager::process_spawns()
	{
		for (auto& t_entity : m_frameSpawns)
		{
			for (auto& t_listHolder : m_systemEntityViewLists)
			{
				t_listHolder->on_entity_added(*t_entity);
			}
			m_entities.emplace(t_entity->get_id(), std::move(t_entity));
		}
		m_frameSpawns.clear();
	}

	void entity_manager::process_unspawns()
	{
		for (auto const id : m_frameUnspawns)
		{
			m_unusedEntityIds.emplace_back(id);
			auto const it = m_entities.find(id);
			if (it != m_entities.end())
			{
				for (auto& t_listHolder : m_systemEntityViewLists)
				{
					t_listHolder->on_entity_removed(*it->second);
				}
				m_entities.erase(it);
			}
		}
		m_frameUnspawns.clear();
	}
}