#include "vob/aoe/debug/DebugUiSystem.h"


namespace vob::aoedb
{
	void DebugUiSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_debugUiCtx.init(a_wdar);
		m_gameInputCtx.init(a_wdar);
	}

	void DebugUiSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto& debugUiCtx = m_debugUiCtx.get(a_wdap);
		for (auto const eventId : m_gameInputCtx.get(a_wdap).getEvents())
		{
			if (eventId == debugUiCtx.toggleDisplayEventId)
			{
				debugUiCtx.isDisplayed = !debugUiCtx.isDisplayed;
			}
		}
	}
}
