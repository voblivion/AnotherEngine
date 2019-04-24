#include <aoe/time/TimeSystem.h>

namespace aoe
{
	namespace time
	{
		// Public
		TimeSystem::TimeSystem(ecs::WorldDataProvider& a_worldDataProvider)
			: m_worldTime{
				*a_worldDataProvider.getWorldComponent<TimeComponent>() }
		{}

		void TimeSystem::update() const
		{
			auto const currentTime = Clock::now();
			if(m_worldTime.m_frameStartTime != TimePoint{})
			{
				m_worldTime.m_frameDuration = currentTime
					- m_worldTime.m_frameStartTime;
				m_worldTime.m_elapsedTime = m_worldTime.m_frameDuration.count();
			}
			m_worldTime.m_frameStartTime = currentTime;
		}
	}
}