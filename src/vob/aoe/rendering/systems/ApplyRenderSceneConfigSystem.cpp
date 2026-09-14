#include "vob/aoe/rendering/systems/ApplyRenderSceneConfigSystem.h"

#include "vob/aoe/rendering/RenderSceneResourcesUtils.h"


namespace vob::aoegl
{
	void ApplyRenderSceneConfigSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_windowCtx.init(a_wdar);
		m_gpuDeleteQueueCtx.init(a_wdar);
		m_renderSceneCtx.init(a_wdar);
	}

	void ApplyRenderSceneConfigSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto const windowSize = m_windowCtx.get(a_wdap).window.get().getSize();
		if (windowSize.x <= 0 || windowSize.y <= 0)
		{
			return;
		}

		auto& renderSceneCtx = m_renderSceneCtx.get(a_wdap);
		auto const& config = renderSceneCtx.config.get();
		updateGpuRenderSceneResources(
			renderSceneCtx.appliedConfig,
			renderSceneCtx.appliedWindowSize,
			config,
			windowSize,
			m_gpuDeleteQueueCtx.get(a_wdap).deleteQueue.get(),
			renderSceneCtx.resources);

		renderSceneCtx.appliedConfig = config;
		renderSceneCtx.appliedWindowSize = windowSize;
	}
}
