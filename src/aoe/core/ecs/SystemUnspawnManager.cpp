#include <vob/aoe/core/ecs/SystemUnspawnManager.h>

namespace vob::aoe::ecs
{
	// Public
	SystemUnspawnManager::SystemUnspawnManager(
		std::pmr::vector<EntityId>& a_frameUnspawns)
		: m_frameUnspawns{ a_frameUnspawns }
	{}

	void SystemUnspawnManager::unspawn(EntityId const a_id)
	{
		std::lock_guard<std::mutex> t_lock{m_mutex};
		m_frameUnspawns.emplace_back(a_id);
	}
}