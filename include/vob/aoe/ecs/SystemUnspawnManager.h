#pragma once

#include <mutex>
#include <vector>

#include <vob/aoe/api.h>
#include <vob/aoe/ecs/EntityId.h>

namespace vob::aoecs
{
	class SystemUnspawnManager
	{
	public:
		// Constructors
		VOB_AOE_API explicit SystemUnspawnManager(std::vector<EntityId>& a_frameUnspawns);

		// Methods
		VOB_AOE_API void unspawn(EntityId const a_id);

	private:
		// Attributes
		std::mutex m_mutex;
		std::vector<EntityId>& m_frameUnspawns;
	};
}