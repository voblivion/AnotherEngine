#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/ImGuiContext.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include <vob/aoe/window/WindowInputContext.h>
#include <vob/aoe/window/WindowContext.h>


namespace vob::aoegl
{
	class VOB_AOE_API PrepareImGuiFrameSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);
		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoewi::WindowInputContext> m_windowInputContext;
		aoeng::EcsWorldContextRef<aoewi::WindowContext> m_windowContext;
		aoeng::EcsWorldContextRef<ImGuiContext> m_imGuiContext;
	};
}
