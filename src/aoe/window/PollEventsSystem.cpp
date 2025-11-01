#include <vob/aoe/window/PollEventsSystem.h>


namespace vob::aoewi
{
	void PollEventsSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_windowContext.init(a_wdar);
	}

	void PollEventsSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdp) const
	{
		m_windowContext.get(a_wdp).window.get().pollEvents();
	}
}
