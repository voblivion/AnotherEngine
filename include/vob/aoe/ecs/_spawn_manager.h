#pragma once

#include <mutex>
#include <vector>

#include <vob/aoe/api.h>
#include <vob/aoe/ecs/_entity.h>

namespace vob::_aoecs
{
	class VOB_AOE_API spawn_manager
	{
	public:
		// Constructors
		explicit spawn_manager(
			std::vector<std::unique_ptr<entity>>& a_frameSpawns, std::vector<entity_id>& a_unusedEntityIds);

		// Methods
		entity& spawn(component_manager a_componentManager);

	private:
		// Attributes
		entity_id::value_type m_nextEntityIdValue{};
		std::vector<std::unique_ptr<entity>>& m_frameSpawns;
		std::vector<entity_id>& m_unusedEntityIds;
	};
}