#include <vob/aoe/input/InputBinding.h>


namespace vob::aoein
{
	void TwoAxesDifferenceInputValueBinding::processEvent(aoewi::WindowEvent const& a_windowEvent)
	{
		m_negative->processEvent(a_windowEvent);
		m_positive->processEvent(a_windowEvent);
	}

	void TwoAxesDifferenceInputValueBinding::update(aoewi::IWindow const& a_window, float a_dt)
	{
		m_negative->update(a_window, a_dt);
		m_positive->update(a_window, a_dt);
	}

	void GamepadAxisInputValueBinding::update(aoewi::IWindow const& a_window, [[maybe_unused]] float a_dt)
	{
		if (a_window.isGamepadPresent(m_gamepadIndex))
		{
			auto const offset = a_window.getGamepadAxisValue(m_gamepadIndex, m_axis) - m_rest;
			auto const sign = static_cast<float>((0.0f < offset) - (offset < 0.0f));
			auto const range = offset > 0.0f ? 1.0f - m_rest : 1.0f + m_rest;
			auto const travel = range > 0.0f ? std::abs(offset) / range : 0.0f;
			m_value = sign * std::max(0.0f, travel - m_deadZone) / (1.0f - m_deadZone);
			if (m_isInverted)
			{
				m_value = -m_value;
			}
		}
		else
		{
			m_value = 0.0f;
		}
	}

	bool GamepadButtonEventBinding::update(aoewi::IWindow const& a_window, [[maybe_unused]] float a_dt)
	{
		auto const wasPressed = m_isPressed;
		if (a_window.isGamepadPresent(m_gamepadIndex))
		{
			m_isPressed = a_window.isGamepadButtonPressed(m_gamepadIndex, m_button);
		}

		return m_isPressed && !wasPressed;
	}

	bool GamepadAxisEventBinding::update(aoewi::IWindow const& a_window, [[maybe_unused]] float a_dt)
	{
		auto const wasPressed = m_isPressed;
		if (a_window.isGamepadPresent(m_gamepadIndex))
		{
			auto const value = a_window.getGamepadAxisValue(m_gamepadIndex, m_axis);
			m_isPressed = (m_isInverted ? -value : value) > m_threshold;
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
