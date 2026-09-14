#pragma once

#include "vob/aoe/rendering/contexts/GpuDeleteQueueContext.h"
#include "vob/aoe/rendering/contexts/RenderSceneContext.h"

#include "vob/aoe/engine/EcsWorldDataAccess.h"
#include "vob/aoe/window/WindowContext.h"


namespace vob::aoegl
{
	class ApplyRenderSceneConfigSystem
	{
	  public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);
		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	  private:
		aoeng::EcsWorldContextRef<aoewi::WindowContext const> m_windowCtx;
		aoeng::EcsWorldContextRef<GpuDeleteQueueContext> m_gpuDeleteQueueCtx;
		aoeng::EcsWorldContextRef<RenderSceneContext> m_renderSceneCtx;
	};
}
