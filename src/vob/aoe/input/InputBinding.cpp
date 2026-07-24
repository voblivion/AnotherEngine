#include <vob/aoe/input/InputBinding.h>


namespace vob::aoein
{
	bool GamepadButtonEventBinding::update(aoewi::IWindow const& a_window, [[maybe_unused]] float a_dt)
	{
		auto const wasPressed = m_isPressed;
		if (a_window.isGamepadPresent(m_gamepadIndex))
		{
			m_isPressed = a_window.isGamepadButtonPressed(m_gamepadIndex, m_button);
		}

		return m_isPressed && !wasPressed;
	}

	bool KeyboardKeyEventBinding::processEvent(aoewi::WindowEvent const& a_windowEvent)
	{
		if (auto const* keyboardKeyEvent = std::get_if<aoewi::KeyEvent>(&a_windowEvent))
		{
			return keyboardKeyEvent->key == m_key && keyboardKeyEvent->action == m_action;
		}

		return false;
	}
}
