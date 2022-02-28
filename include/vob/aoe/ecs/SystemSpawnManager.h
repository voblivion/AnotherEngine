#pragma once

#include <mutex>
#include <vector>

#include <vob/aoe/api.h>
#include <vob/aoe/ecs/entity.h>

namespace vob::aoecs
{
	class VOB_AOE_API SystemSpawnManager
	{
	public:
		// Constructors
		explicit SystemSpawnManager(std::vector<std::unique_ptr<entity>>& a_frameSpawns);

		// Methods
		entity& spawn(component_manager a_componentManager);

	private:
		// Attributes
		entity_id::value_type m_nextEntityIdValue{};
		std::mutex m_mutex{};
		std::vector<std::unique_ptr<entity>>& m_frameSpawns;
	};
}