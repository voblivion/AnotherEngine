#pragma once

#include <mutex>
#include <vector>

#include <vob/aoe/api.h>
#include <vob/aoe/core/ecs/EntityId.h>

namespace vob::aoe::ecs
{
	class SystemUnspawnManager
	{
	public:
		// Constructors
		VOB_AOE_API explicit SystemUnspawnManager(
			std::pmr::vector<EntityId>& a_frameUnspawns);

		// Methods

		VOB_AOE_API void unspawn(EntityId const a_id);

	private:
		// Attributes
		std::mutex m_mutex;
		std::pmr::vector<EntityId>& m_frameUnspawns;
	};
}