#pragma once

#include <vob/aoe/input/InputBindings.h>

#include <vob/aoe/window/Window.h>

#include <algorithm>


namespace vob::aoein
{
	struct AInputValueBinding
	{
		virtual ~AInputValueBinding() = default;

		virtual float getValue() const = 0;

		virtual void processEvent(aoewi::WindowEvent const& a_windowEvent) {};

		virtual void update(aoewi::IWindow const& a_window, float a_dt) {};
	};

	struct MouseAxisInputValueBinding : public AInputValueBinding
	{
	public:
		explicit MouseAxisInputValueBinding(Mouse::Axis a_axis)
			: m_axis{ a_axis }
		{
		}

		float getValue() const override
		{
			return m_value;
		}

		void processEvent(aoewi::WindowEvent const& a_windowEvent) override
		{
			if (auto const* mouseMoveEvent = std::get_if<aoewi::MouseMoveEvent>(&a_windowEvent))
			{
				switch (m_axis)
				{
				case Mouse::Axis::X:
					m_value = static_cast<float>(mouseMoveEvent->position.x);
					break;
				case Mouse::Axis::Y:
					m_value = static_cast<float>(mouseMoveEvent->position.y);
					break;
				}
			}
		}

	private:
		Mouse::Axis m_axis;
		float m_value = 0.0f;
	};

	struct AWindowSwitchInputValueBinding : public AInputValueBinding
	{
	public:
		explicit AWindowSwitchInputValueBinding(float a_restValue = 0.0f, float a_pressedValue = 1.0f, float a_speed = 0.0f)
			: m_restValue{ a_restValue }
			, m_pressedValue{ a_pressedValue }
			, m_speed{ a_speed }
		{
			assert(m_speed >= 0.0f);
		}

		float getValue() const override
		{
			return m_value;
		}

		void update(aoewi::IWindow const& a_window, float a_dt) override
		{
			if (m_speed > 0.0f)
			{
				m_value += (m_isPressed ? m_speed : -m_speed) * a_dt;
				m_value = std::clamp(m_value, std::min(m_restValue, m_pressedValue), std::max(m_restValue, m_pressedValue));
			}
			else
			{
				m_value = m_isPressed ? m_pressedValue : m_restValue;
			}
		}

	protected:
		bool m_isPressed = false;

	private:
		float m_restValue = 0.0f;
		float m_pressedValue = 0.0f;
		float m_speed = 0.0f;
		float m_value = 0.0f;
	};

	struct MouseButtonInputValueBinding : public AWindowSwitchInputValueBinding
	{
	public:
		explicit MouseButtonInputValueBinding(Mouse::Button a_button, float a_restValue = 0.0f, float a_pressedValue = 1.0f, float a_speed = 0.0f)
			: AWindowSwitchInputValueBinding{ a_restValue, a_pressedValue, a_speed }
			, m_button{ a_button }
		{
		}

		void processEvent(aoewi::WindowEvent const& a_windowEvent) override
		{
			if (auto const* mouseButtonEvent = std::get_if<aoewi::MouseButtonEvent>(&a_windowEvent))
			{
				m_isPressed = mouseButtonEvent->button == m_button && mouseButtonEvent->pressed;
			}
		}

	private:
		Mouse::Button m_button;
	};

	struct KeyboardKeyInputValueBinding : public AWindowSwitchInputValueBinding
	{
	public:
		explicit KeyboardKeyInputValueBinding(Keyboard::Key a_key, float a_restValue = 0.0f, float a_pressedValue = 1.0f, float a_speed = 0.0f)
			: AWindowSwitchInputValueBinding{ a_restValue, a_pressedValue, a_speed }
			, m_key{ a_key }
		{
		}

		void processEvent(aoewi::WindowEvent const& a_windowEvent) override
		{
			if (auto const* keyEvent = std::get_if<aoewi::KeyEvent>(&a_windowEvent))
			{
				if (keyEvent->key == m_key)
				{
					m_isPressed = keyEvent->action != aoewi::KeyEvent::Action::Release;
				}
			}
		}

	private:
		Keyboard::Key m_key;
	};

	struct UpDownInputValueBinding : public AInputValueBinding
	{
	public:
		explicit UpDownInputValueBinding(
			std::shared_ptr<AInputValueBinding> a_up, std::shared_ptr<AInputValueBinding> a_down)
			: m_up{ std::move(a_up) }
			, m_down{ std::move(a_down) }
		{
		}

		float getValue() const override
		{
			return m_up->getValue() - m_down->getValue();
		}

		void processEvent(aoewi::WindowEvent const& a_windowEvent) override
		{
			m_up->processEvent(a_windowEvent);
			m_down->processEvent(a_windowEvent);
		};

		void update(aoewi::IWindow const& a_window, float a_dt) override
		{
			m_up->update(a_window, a_dt);
			m_down->update(a_window, a_dt);
		}

	private:
		std::shared_ptr<AInputValueBinding> m_up;
		std::shared_ptr<AInputValueBinding> m_down;
	};

	struct GamepadAxisInputValueBinding : public AInputValueBinding
	{
	public:
		explicit GamepadAxisInputValueBinding(int32_t a_gamepadIndex, Gamepad::Axis a_axis, float a_deadZone = 0.0f)
			: m_gamepadIndex{ a_gamepadIndex }
			, m_axis{ a_axis }
			, m_deadZone{ a_deadZone }
		{
		}

		float getValue() const override
		{
			return m_value;
		}

		void update(aoewi::IWindow const& a_window, float a_dt) override
		{
			if (a_window.isGamepadPresent(m_gamepadIndex))
			{
				m_value = a_window.getGamepadAxisValue(m_gamepadIndex, m_axis);
				auto const sign = (0.0f < m_value) - (m_value < 0.0f);
				m_value = sign * (std::max(m_deadZone, std::abs(m_value)) - m_deadZone) / (1.0f - m_deadZone);
			}
			else
			{
				m_value = 0.0f;
			}
		}

	private:
		int32_t m_gamepadIndex = 0;
		Gamepad::Axis m_axis;
		float m_deadZone = 0.0f;
		float m_value = 0.0f;
	};

	struct GamepadButtonInputValueBinding : public AWindowSwitchInputValueBinding
	{
	public:
		explicit GamepadButtonInputValueBinding(
			int32_t a_gamepadIndex, Gamepad::Button a_button, float a_restValue = 0.0f, float a_pressedValue = 1.0f, float a_speed = 0.0f)
			: AWindowSwitchInputValueBinding{ a_restValue, a_pressedValue, a_speed }
			, m_gamepadIndex{ a_gamepadIndex }
			, m_button{ a_button }
		{
		}

		void update(aoewi::IWindow const& a_window, float a_dt) override
		{
			if (a_window.isGamepadPresent(m_gamepadIndex))
			{
				m_isPressed = a_window.isGamepadButtonPressed(m_gamepadIndex, m_button);
			}

			AWindowSwitchInputValueBinding::update(a_window, a_dt);
		}

	private:
		int32_t m_gamepadIndex = 0;
		Gamepad::Button m_button;
	};

	struct DerivedInputValueBinding : public AInputValueBinding
	{
	public:
		explicit DerivedInputValueBinding(
			std::shared_ptr<AInputValueBinding> a_valueBinding,
			float a_sensitivity = 1.0f)
			: m_valueBinding{ std::move(a_valueBinding) }
			, m_sensitivity{ a_sensitivity }
		{}

		float getValue() const override
		{
			return m_derivedValue;
		}

		void processEvent(aoewi::WindowEvent const& a_windowEvent) override
		{
			m_valueBinding->processEvent(a_windowEvent);
		};

		void update(aoewi::IWindow const& a_window, float a_dt) override
		{
			m_valueBinding->update(a_window, a_dt);

			auto newValue = m_valueBinding->getValue();
			m_derivedValue = (newValue - m_prevValue) * m_sensitivity  / a_dt;
			m_prevValue = newValue;
		}

	private:
		std::shared_ptr<AInputValueBinding>	m_valueBinding;
		float m_sensitivity = 0.0f;
		float m_derivedValue = 0.0f;
		float m_prevValue = 0.0f;
	};

	struct AInputEventBinding
	{
		virtual ~AInputEventBinding() = default;

		virtual bool processEvent(aoewi::WindowEvent const& a_windowEvent) { return false; };

		virtual bool update(aoewi::IWindow const& a_window, float a_dt) { return false; }
	};

	struct MouseButtonEventBinding : public AInputEventBinding
	{
	public:
		explicit MouseButtonEventBinding(aoein::Mouse::Button a_button, bool a_pressed = true)
			: m_button{ a_button }
			, m_pressed{ a_pressed }
		{
		}

		bool processEvent(aoewi::WindowEvent const& a_windowEvent) override
		{
			if (auto const* mouseButtonEvent = std::get_if<aoewi::MouseButtonEvent>(&a_windowEvent))
			{
				return mouseButtonEvent->button == m_button && mouseButtonEvent->pressed == m_pressed;
			}
			
			return false;
		}

	private:
		aoein::Mouse::Button m_button;
		bool m_pressed;
	};

	struct MouseScrollEventBinding : public AInputEventBinding
	{
	public:
		explicit MouseScrollEventBinding(bool a_up = true)
			: m_up{ a_up }
		{
		}

		bool processEvent(aoewi::WindowEvent const& a_windowEvent) override
		{
			if (auto const* mouseScrollEvent = std::get_if<aoewi::MouseScrollEvent>(&a_windowEvent))
			{
				return (mouseScrollEvent->move.y > 0) == m_up;
			}

			return false;
		}

	private:
		bool m_up;
	};


	struct GamepadButtonEventBinding : public AInputEventBinding
	{
	public:
		explicit GamepadButtonEventBinding(int32_t a_gamepadIndex, Gamepad::Button a_button)
			: m_gamepadIndex{ a_gamepadIndex }
			, m_button{ a_button }
		{
		}

		bool update(aoewi::IWindow const& a_window, float a_dt) override;

	private:
		int32_t m_gamepadIndex = 0;
		Gamepad::Button m_button;
		bool m_isPressed = false;
	};
	// TODO
}
