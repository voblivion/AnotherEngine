#include <vob/aoe/window/SwapBuffersSystem.h>


namespace vob::aoewi
{
	void SwapBuffersSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_windowContext.init(a_wdar);
	}

	void SwapBuffersSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdp) const
	{
		m_windowContext.get(a_wdp).window.get().swapBuffers();
	}
}
