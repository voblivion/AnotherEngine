#include <vob/aoe/spacetime/FixedRateTimeSystem.h>

#include "imgui.h"


namespace vob::aoest
{
	void FixedRateTimeSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_timeContext.init(a_wdar);
		m_fixedRateTimeContext.init(a_wdar);
	}

	void FixedRateTimeSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		static bool k_isPaused = true;

		ImGui::Begin("Fixed Rate");
		ImGui::Checkbox("Is Paused", &k_isPaused);
		static bool k_step = false;
		if (ImGui::Button("Step"))
		{
			k_step = true;
			k_isPaused = true;
		}
		ImGui::End();

		// TODO: put offset somewhere to reset everything without changing world?
		auto& timeContext = m_timeContext.get(a_wdap);
		auto& fixedRateTimeContext = m_fixedRateTimeContext.get(a_wdap);

		if (k_isPaused)
		{
			fixedRateTimeContext.tickStartTime = timeContext.tickStartTime;
		}

		if (timeContext.tickStartTime <= fixedRateTimeContext.tickStartTime + fixedRateTimeContext.tickDuration && !k_step)
		{
			return;
		}
		k_step = false;

		fixedRateTimeContext.tickStartTime += fixedRateTimeContext.tickDuration;
		++fixedRateTimeContext.tickIndex;
	}
}
