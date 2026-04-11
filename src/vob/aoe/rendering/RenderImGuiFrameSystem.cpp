#include <vob/aoe/rendering/RenderImGuiFrameSystem.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"


namespace vob::aoegl
{
	void RenderImGuiFrameSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_windowContext.init(a_wdar);
		m_imGuiContext.init(a_wdar);
	}

	void RenderImGuiFrameSystem::execute([[maybe_unused]] aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		ImGui::Render();
		if (ImGui::GetDrawData()->CmdListsCount > 0)
		{
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}
	}
}
