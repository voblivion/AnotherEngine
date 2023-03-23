#pragma once

#include <mutex>
#include <vector>

#include <vob/aoe/api.h>
#include <vob/aoe/ecs/_entity_id.h>

namespace vob::_aoecs
{
	class unspawn_manager
	{
	public:
		// Constructors
		VOB_AOE_API explicit unspawn_manager(std::vector<entity_id>& a_frameUnspawns);

		// Methods
		VOB_AOE_API void unspawn(entity_id const a_id);

	private:
		// Attributes
		std::mutex m_mutex;
		std::vector<entity_id>& m_frameUnspawns;
	};
}