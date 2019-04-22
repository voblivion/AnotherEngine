#pragma once

#include <mutex>
#include <vector>

#include <aoe/Config.h>
#include <aoe/core/EntityId.h>

namespace aoe
{
	namespace core
	{
		class SystemUnspawnManager
		{
		public:
			// Constructors
			AOE_API explicit SystemUnspawnManager(
				std::pmr::vector<EntityId>& a_frameUnspawns);

			// Methods

			AOE_API void unspawn(EntityId const a_id);

		private:
			// Attributes
			std::mutex m_mutex;
			std::pmr::vector<EntityId>& m_frameUnspawns;
		};
	}
}