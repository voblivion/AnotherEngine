#include <vob/aoe/common/time/TimeSystem.h>

namespace vob::aoe::common
{
	// Public
	TimeSystem::TimeSystem(aoecs::WorldDataProvider& a_worldDataProvider)
		: m_worldTimeComponent{ *a_worldDataProvider.getWorldComponent<WorldTimeComponent>() }
	{}

	void TimeSystem::update() const
	{
		auto const currentTime = Clock::now();
		if(m_worldTimeComponent.m_frameStartTime != TimePoint{})
		{
			m_worldTimeComponent.m_frameDuration = currentTime
				- m_worldTimeComponent.m_frameStartTime;
			m_worldTimeComponent.m_elapsedTime = m_worldTimeComponent.m_frameDuration;
		}
		m_worldTimeComponent.m_frameStartTime = currentTime;
	}
}