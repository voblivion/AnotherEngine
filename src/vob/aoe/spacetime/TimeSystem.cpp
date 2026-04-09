#include <vob/aoe/spacetime/TimeSystem.h>

#include <chrono>


namespace vob::aoest
{
	void TimeSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_timeContext.init(a_wdar);
	}

	void TimeSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto& timeContext = m_timeContext.get(a_wdap);
		auto const currentTime = std::chrono::high_resolution_clock::now();

		timeContext.elapsedTime = currentTime - timeContext.tickStartTime;
		timeContext.tickStartTime = currentTime;
	}
}
