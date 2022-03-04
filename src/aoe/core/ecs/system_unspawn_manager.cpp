#include <vob/aoe/ecs/system_unspawn_manager.h>

namespace vob::aoecs
{
	// Public
	system_unspawn_manager::system_unspawn_manager(std::vector<entity_id>& a_frameUnspawns)
		: m_frameUnspawns{ a_frameUnspawns }
	{}

	void system_unspawn_manager::unspawn(entity_id const a_id)
	{
		std::lock_guard<std::mutex> t_lock{ m_mutex };
		m_frameUnspawns.emplace_back(a_id);
	}
}