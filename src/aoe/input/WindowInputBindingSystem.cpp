#include <vob/aoe/input/WindowInputBindingSystem.h>

#include <imgui.h>


namespace vob::aoein
{
	void WindowInputBindingSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto const dt = std::chrono::duration<float>(m_timeContext.get(a_wdap).elapsedTime).count();
		auto const& window = m_windowContext.get(a_wdap).window.get();
		auto& inputBindingCtx = m_inputBindingContext.get(a_wdap);
		auto& gameInputCtx = m_gameInputContext.get(a_wdap);
		gameInputCtx.flushEvents();

		// Process events
		for (auto const& polledEvent : window.getPolledEvents())
		{
			for (auto& inputValueBinding : inputBindingCtx.values)
			{
				inputValueBinding.second->processEvent(polledEvent);
			}

			for (auto& inputEventBinding : inputBindingCtx.events)
			{
				if (inputEventBinding.second->processEvent(polledEvent))
				{
					gameInputCtx.addEvent(inputEventBinding.first);
				}
			}
		}

		// Process time and gamepad changes
		for (auto& inputValueBinding : inputBindingCtx.values)
		{
			inputValueBinding.second->update(window, dt);
			gameInputCtx.setValue(inputValueBinding.first, inputValueBinding.second->getValue());
		}

		for (auto& inputEventBinding : inputBindingCtx.events)
		{
			if (inputEventBinding.second->update(window, dt))
			{
				gameInputCtx.addEvent(inputEventBinding.first);
			}
		}

		ImGui::Begin("Inputs");
		{
			float v0 = gameInputCtx.getValue(4);
			ImGui::InputFloat("Input 4", &v0);
		}
		ImGui::End();
	}
}
