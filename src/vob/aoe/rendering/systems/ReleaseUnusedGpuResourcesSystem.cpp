#include "vob/aoe/rendering/systems/ReleaseUnusedGpuResourcesSystem.h"


namespace vob::aoegl
{
	void ReleaseUnusedGpuResourcesSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_gpuDeleteQueueContext.init(a_wdar);
	}

	void ReleaseUnusedGpuResourcesSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		m_gpuDeleteQueueContext.get(a_wdap).deleteQueue.get().drain();
	}
}
