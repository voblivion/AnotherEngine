#include "vob/aoe/spacetime/DebugTimeSystem.h"

#include "imgui.h"

#include <chrono>


namespace vob::aoest
{
	void DebugTimeSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_timeContext.init(a_wdar);
	}

	void DebugTimeSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		if (ImGui::Begin("Debug Time"))
		{
			ImGui::BeginDisabled();
			auto frameTime = std::chrono::duration_cast<std::chrono::duration<float>>(m_timeContext.get(a_wdap).elapsedTime).count();
			ImGui::InputFloat("Frame Time", &frameTime);
			auto fps = 1.0f / frameTime;
			ImGui::InputFloat("FPS", &fps);
			ImGui::EndDisabled();
		}
		ImGui::End();
	}
}
