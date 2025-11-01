#include <vob/aoe/window/WindowInputSystem.h>


namespace vob::aoewi
{
	namespace
	{
		void processEvent(
			KeyEvent const& a_keyEvent,
			aoewi::WindowInputContext const& a_windowInputContext,
			aoein::Inputs& a_inputs)
		{
			if (a_windowInputContext.shouldIgnoreKeyboardKeyEvents)
			{
				return;
			}

			auto& keyState = a_inputs.keyboard.keys[a_keyEvent.key];
			keyState.updateState(a_keyEvent.action != KeyEvent::Action::Release);
		}

		void processEvent(
			TextEvent const& a_textEvent,
			aoewi::WindowInputContext const& a_windowInputContext,
			aoein::Inputs& a_inputs)
		{

		}

		void processEvent(
			MouseMoveEvent const& a_mouseMoveEvent,
			aoewi::WindowInputContext const& a_windowInputContext,
			aoein::Inputs& a_inputs)
		{
			a_inputs.mouse.axes[aoein::Mouse::Axis::X].updateState(
				static_cast<float>(a_mouseMoveEvent.position.x));
			a_inputs.mouse.axes[aoein::Mouse::Axis::Y].updateState(
				static_cast<float>(a_mouseMoveEvent.position.y));
		}

		void processEvent(
			MouseButtonEvent const& a_mouseButtonEvent,
			aoewi::WindowInputContext const& a_windowInputContext,
			aoein::Inputs& a_inputs)
		{
			if (a_windowInputContext.shouldIgnoreMouseButtonEvents)
			{
				return;
			}

			auto& buttonState = a_inputs.mouse.buttons[a_mouseButtonEvent.button];
			buttonState.updateState(a_mouseButtonEvent.pressed);
		}

		void processEvent(
			MouseScrollEvent const& a_mouseScrollEvent,
			aoewi::WindowInputContext const& a_windowInputContext,
			aoein::Inputs& a_inputs)
		{
			auto button = a_mouseScrollEvent.move.y > 0.0f
				? aoein::Mouse::Button::ScrollUp : aoein::Mouse::Button::ScrollDown;
			auto& buttonState = a_inputs.mouse.buttons[button];
			buttonState.updateState(true);
		}

		void processEvent(
			MouseHoverEvent const& a_mouseHoverEvent,
			aoewi::WindowInputContext const& a_windowInputContext,
			aoein::Inputs& a_inputs)
		{

		}

		void processEvents(
			IWindow& a_window,
			const aoewi::WindowInputContext& a_windowInputContext,
			aoein::Inputs& a_inputs)
		{
			for (auto const& polledEvent : a_window.getPolledEvents())
			{
				std::visit([&a_windowInputContext, &a_inputs](auto const& aEvent) {
					processEvent(aEvent, a_windowInputContext, a_inputs);
					}, polledEvent);
			}
		}
	}

	void WindowInputSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_windowContext.init(a_wdar);
		m_windowInputContext.init(a_wdar);
		m_inputs.init(a_wdar);
	}

	void WindowInputSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto& window = m_windowContext.get(a_wdap).window.get();
		auto& inputs = m_inputs.get(a_wdap);

		inputs.resetChangedStates();

		processEvents(window, m_windowInputContext.get(a_wdap), inputs);

		for (int32_t g = 0; g < aoein::k_maxGamepadCount; ++g)
		{
			auto& gamepad = inputs.gamepads[g];

			const bool isGamepadPresent = window.isGamepadPresent(g);
			gamepad.isConnected.updateState(isGamepadPresent);
			if (isGamepadPresent)
			{
				for (auto button : gamepad.buttons.keys())
				{
					gamepad.buttons[button].updateState(window.isGamepadButtonPressed(g, button));
				}
				for (auto axis : gamepad.axes.keys())
				{
					gamepad.axes[axis].updateState(window.getGamepadAxisValue(g, axis));
				}
			}
		}
	}
}
