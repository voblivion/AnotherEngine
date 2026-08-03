#include "vob/aoe/rendering/systems/RenderImGuiFrameSystem.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"


namespace vob::aoegl
{
	void RenderImGuiFrameSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_windowContext.init(a_wdar);
		m_imGuiContext.init(a_wdar);
		m_renderProfilingCtx.init(a_wdar);
		m_gameInputCtx.init(a_wdar);
	}

	void RenderImGuiFrameSystem::execute([[maybe_unused]] aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto& imGuiCtx = m_imGuiContext.get(a_wdap);
		for (auto const eventId : m_gameInputCtx.get(a_wdap).getEvents())
		{
			if (eventId == imGuiCtx.toggleDisplayEventId)
			{
				imGuiCtx.isDisplayed = !imGuiCtx.isDisplayed;
			}
		}

		ImGui::Render();
		if (ImGui::GetDrawData()->CmdListsCount > 0)
		{
			if (imGuiCtx.isDisplayed)
			{
				auto& renderProfilingCtx = m_renderProfilingCtx.get(a_wdap);
				VOB_AOE_GPU_TIMER_SCOPE(renderProfilingCtx.gpuProfiler, "ImGui");
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			}
		}
	}
}
