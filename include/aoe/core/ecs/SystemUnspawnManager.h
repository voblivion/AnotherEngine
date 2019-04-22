#pragma once

#include <mutex>
#include <vector>

#include <aoe/core/Export.h>
#include <aoe/core/ecs/EntityId.h>

namespace aoe
{
	namespace ecs
	{
		class SystemUnspawnManager
		{
		public:
			// Constructors
			AOE_CORE_API explicit SystemUnspawnManager(
				std::pmr::vector<EntityId>& a_frameUnspawns);

			// Methods

			AOE_CORE_API void unspawn(EntityId const a_id);

		private:
			// Attributes
			std::mutex m_mutex;
			std::pmr::vector<EntityId>& m_frameUnspawns;
		};
	}
}