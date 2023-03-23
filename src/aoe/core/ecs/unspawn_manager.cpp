#include <vob/aoe/ecs/_unspawn_manager.h>


namespace vob::_aoecs
{
	// Public
	unspawn_manager::unspawn_manager(std::vector<entity_id>& a_frameUnspawns)
		: m_frameUnspawns{ a_frameUnspawns }
	{}

	void unspawn_manager::unspawn(entity_id const a_id)
	{
		std::lock_guard<std::mutex> lock{ m_mutex };
		m_frameUnspawns.emplace_back(a_id);
	}
}