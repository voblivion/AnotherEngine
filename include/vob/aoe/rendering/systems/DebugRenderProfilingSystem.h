#pragma once

#include "vob/aoe/rendering/contexts/RenderProfilingContext.h"

#include "vob/aoe/debug/DebugUiContext.h"
#include "vob/aoe/engine/EcsWorldDataAccess.h"


namespace vob::aoegl
{
	class DebugRenderProfilingSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<RenderProfilingContext> m_renderProfilingCtx;
		aoeng::EcsWorldContextRef<aoedb::DebugUiContext const> m_debugUiCtx;
	};
}
