#include <vob/aoe/spacetime/FixedRateLimitingSystem.h>

#include <chrono>
#include <thread>


namespace vob::aoest
{
	void FixedRateLimitingSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_fixedRateTimeContext.init(a_wdar);
	}

	void FixedRateLimitingSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto& fixedRateTimeContext = m_fixedRateTimeContext.get(a_wdap);
		auto const currentTime = std::chrono::high_resolution_clock::now();
		auto const nextFixedRateTickStartTime = fixedRateTimeContext.tickStartTime + fixedRateTimeContext.tickDuration;
		if (currentTime < nextFixedRateTickStartTime)
		{
			std::this_thread::sleep_for(nextFixedRateTickStartTime - currentTime);
		}
	}
}
