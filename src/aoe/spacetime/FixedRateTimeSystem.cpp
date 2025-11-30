#include <vob/aoe/spacetime/FixedRateTimeSystem.h>


namespace vob::aoest
{
	void FixedRateTimeSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_timeContext.init(a_wdar);
		m_fixedRateTimeContext.init(a_wdar);
	}

	void FixedRateTimeSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		// TODO: put offset somewhere to reset everything without changing world?
		auto& timeContext = m_timeContext.get(a_wdap);
		auto& fixedRateTimeContext = m_fixedRateTimeContext.get(a_wdap);

		if (timeContext.tickStartTime <= fixedRateTimeContext.tickStartTime + fixedRateTimeContext.tickDuration)
		{
			return;
		}

		fixedRateTimeContext.tickStartTime += fixedRateTimeContext.tickDuration;
		++fixedRateTimeContext.tickIndex;
	}
}
