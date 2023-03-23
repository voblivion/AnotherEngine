#include <vob/aoe/common/time/TimeSystem.h>

namespace vob::aoe::common
{
	// Public
	TimeSystem::TimeSystem(aoecs::world_data_provider& a_wdp)
		: m_worldTimeComponent{ a_wdp.get_world_component<WorldTimeComponent>() }
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