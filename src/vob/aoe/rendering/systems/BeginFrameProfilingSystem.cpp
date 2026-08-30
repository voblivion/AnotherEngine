#include "vob/aoe/rendering/systems/BeginFrameProfilingSystem.h"


namespace vob::aoegl
{
	void BeginFrameProfilingSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_renderProfilingCtx.init(a_wdar);
	}

	void BeginFrameProfilingSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		pushGpuTimerScope(m_renderProfilingCtx.get(a_wdap).gpuProfiler, "Frame");
	}
}
