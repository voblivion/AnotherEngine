#include "vob/aoe/rendering/systems/EndFrameProfilingSystem.h"


namespace vob::aoegl
{
	void EndFrameProfilingSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_renderProfilingCtx.init(a_wdar);
	}

	void EndFrameProfilingSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		popGpuTimerScope(m_renderProfilingCtx.get(a_wdap).gpuProfiler);
	}
}
