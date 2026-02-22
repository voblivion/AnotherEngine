#include <vob/aoe/spacetime/FixedRateLimitingSystem.h>

#include <tracy/Tracy.hpp>

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
		auto const nextFixedRateTickStartTime = fixedRateTimeContext.tickStartTime + fixedRateTimeContext.tickDuration;

		auto currentTime = std::chrono::high_resolution_clock::now();
		if (currentTime + std::chrono::milliseconds(1) < nextFixedRateTickStartTime)
		{
			std::this_thread::sleep_for(nextFixedRateTickStartTime - currentTime - std::chrono::milliseconds(1));
		}

		while (currentTime < nextFixedRateTickStartTime)
		{
			std::this_thread::yield();
			currentTime = std::chrono::high_resolution_clock::now();
		}
	}
}
