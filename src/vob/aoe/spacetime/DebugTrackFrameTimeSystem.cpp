#include "vob/aoe/spacetime/DebugTrackFrameTimeSystem.h"

#include <chrono>


namespace vob::aoest
{
	void DebugTrackFrameTimeSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
	}

	void DebugTrackFrameTimeSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto const& timeCtx = m_timeContext.get(a_wdap);
		auto& debugFrameTimeCtx = m_debugFrameTimeContext.get(a_wdap);

		debugFrameTimeCtx.frameTime = std::chrono::high_resolution_clock::now() - timeCtx.tickStartTime;
	}
}
