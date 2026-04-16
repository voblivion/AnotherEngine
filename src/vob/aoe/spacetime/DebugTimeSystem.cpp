#include "vob/aoe/spacetime/DebugTimeSystem.h"

#include "imgui.h"

#include <numeric>
#include <array>
#include <chrono>


namespace vob::aoest
{
	void DebugTimeSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_timeContext.init(a_wdar);
	}

	void DebugTimeSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		static std::array<float, 100> k_frameDurations;
		static int32_t k_nextIndex = 0;

		if (ImGui::Begin("Debug Time"))
		{
			ImGui::BeginDisabled();
			auto frameDuration = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(m_timeContext.get(a_wdap).elapsedTime).count();
			k_frameDurations[k_nextIndex++] = frameDuration;
			k_nextIndex = k_nextIndex % k_frameDurations.size();

			auto avgFrameDuration = std::accumulate(k_frameDurations.begin(), k_frameDurations.end(), 0.0f) / k_frameDurations.size();
			ImGui::InputFloat("Frame Duration", &avgFrameDuration);
			auto fps = 1.0f / avgFrameDuration * 1000.0f;
			ImGui::InputFloat("FPS", &fps);
			ImGui::EndDisabled();
		}
		ImGui::End();
	}
}
