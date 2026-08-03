#pragma once

#include "vob/aoe/rendering/contexts/ImGuiContext.h"
#include "vob/aoe/rendering/contexts/RenderProfilingContext.h"

#include "vob/aoe/engine/EcsWorldDataAccess.h"
#include "vob/aoe/input/GameInputContext.h"
#include "vob/aoe/window/WindowContext.h"


namespace vob::aoegl
{
	class RenderImGuiFrameSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);
		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoewi::WindowContext> m_windowContext;
		aoeng::EcsWorldContextRef<ImGuiContext> m_imGuiContext;
		aoeng::EcsWorldContextRef<RenderProfilingContext> m_renderProfilingCtx;
		aoeng::EcsWorldContextRef<aoein::GameInputContext> m_gameInputCtx;
	};
}
