#include <vob/aoe/ecs/_spawn_manager.h>

namespace vob::_aoecs
{
	// Public
	spawn_manager::spawn_manager(
		std::vector<std::unique_ptr<entity>>& a_frameSpawns, std::vector<entity_id>& a_unusedEntityIds)
		: m_frameSpawns{ a_frameSpawns }
		, m_unusedEntityIds{ a_unusedEntityIds }
	{}

	entity& spawn_manager::spawn(component_manager a_componentManager)
	{
		// TODO : ugly code
		entity_id const entityId = [this]()
		{
			if (m_unusedEntityIds.empty())
			{
				return entity_id{ m_nextEntityIdValue++ };
			}
			entity_id const entityId = m_unusedEntityIds.back();
			m_unusedEntityIds.pop_back();
			return entityId;
		}();
		auto& t_entity = m_frameSpawns.emplace_back(std::make_unique<entity>(entityId, a_componentManager));

		return *t_entity;
	}
}