#include <vob/aoe/input/InputBinding.h>


namespace vob::aoein
{
	bool GamepadButtonEventBinding::update(aoewi::IWindow const& a_window, float a_dt)
	{
		auto const wasPressed = m_isPressed;
		if (a_window.isGamepadPresent(m_gamepadIndex))
		{
			m_isPressed = a_window.isGamepadButtonPressed(m_gamepadIndex, m_button);
		}

		return m_isPressed && !wasPressed;
	}
}
