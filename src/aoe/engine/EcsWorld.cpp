#include <vob/aoe/engine/EcsWorld.h>


namespace vob::aoeng
{
	EcsWorld::EcsWorld(std::vector<std::shared_ptr<IEcsSystem>> a_systems, EcsSchedule a_scheduleInfo, entt::registry a_registry)
		: m_systems{ std::move(a_systems) }
		, m_frameJobs{ makeFrameJobs(std::move(a_scheduleInfo)) }
		, m_synchronizedFrameScheduler{ m_frameJobs }
		, m_registry{ std::move(a_registry) }
	{
		EcsWorldDataAccessRegistrar wdac;
		m_systemStates.reserve(a_systems.size());
		for (auto& system : m_systems)
		{
			system->init(wdac);
			m_systemStates.emplace_back(std::make_unique<EcsSystemState>());
		}

		// TODO: move systems to new game architecture
		// TODO: firgure out inter-world communication
	}
}
