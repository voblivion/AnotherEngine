#pragma once

#include <vob/misc/std/enum_map.h>

#include <array>
#include <cstdint>
#include <string_view>


namespace vob::aoein
{
	class AxisInput
	{
	public:
		float getChange() const
		{
			return m_change;
		}

		float getValue() const
		{
			return m_value;
		}

		void updateState(float const a_value)
		{
			m_change = a_value - m_value;
			m_value = a_value;
		}

		void resetChangedState()
		{
			m_change = 0.0f;
		}

	private:
		float m_change = 0.0f;
		float m_value = -1.0f;
	};

	struct SwitchInput
	{
	public:
		bool hasChanged() const
		{
			return m_hasChanged;
		}

		bool isPressed() const
		{
			return m_isPressed;
		}

		void updateState(bool const a_isPressed)
		{
			m_hasChanged = m_isPressed != a_isPressed;
			m_isPressed = a_isPressed;
		}

		void resetChangedState()
		{
			m_hasChanged = false;
		}

		bool wasPressed() const
		{
			return hasChanged() && isPressed();
		}

	private:
		bool m_isPressed = false;
		bool m_hasChanged = false;
	};

	struct Mouse
	{
		enum class Axis
		{
			unkown = -1,
			X = 0,
			Y = 1,
			count
		};

		enum class Button
		{
			Unknown = -1,
			M1 = 0,
			M2,
			M3,
			M4,
			M5,
			M6,
			M7,
			M8,
			// TODO: this probably doesn't belong here
			ScrollUp,
			ScrollDown,
			count,

			// Named Aliases
			Left = M1,
			Right = M2,
			Middle = M3,
			X1 = M4,
			X2 = M5
		};

		SwitchInput hover;
		mistd::enum_map<Axis, AxisInput> axes;
		mistd::enum_map<Button, SwitchInput> buttons;

		void resetChangedStates()
		{
			hover.resetChangedState();

			for (auto& axis : axes)
			{
				axis.resetChangedState();
			}

			for (auto& button : buttons)
			{
				button.resetChangedState();
			}

			// Scroll wheel only provides changes (could be an axis but not great)
			buttons[Button::ScrollUp].updateState(false);
			buttons[Button::ScrollDown].updateState(false);
		}
	};

	struct Keyboard
	{
		enum class Key
		{
			Unknown = -1
			, A = 0
			, B
			, C
			, D
			, E
			, F
			, G
			, H
			, I
			, J
			, K
			, L
			, M
			, N
			, O
			, P
			, Q
			, R
			, S
			, T
			, U
			, V
			, W
			, X
			, Y
			, Z
			, Num0
			, Num1
			, Num2
			, Num3
			, Num4
			, Num5
			, Num6
			, Num7
			, Num8
			, Num9
			, Escape
			, LControl
			, LShift
			, LAlt
			, LSystem
			, RControl
			, RShift
			, RAlt
			, RSystem
			, Menu
			, LBracket
			, RBracket
			, GraveAccent
			, Semicolon
			, Comma
			, Period
			, Quote
			, Slash
			, Backslash
			, Tilde
			, Equal
			, Hyphen
			, Space
			, Enter
			, Backspace
			, Tab
			, PageUp
			, PageDown
			, End
			, Home
			, Insert
			, Delete
			, Add
			, Subtract
			, Multiply
			, Divide
			, Left
			, Right
			, Up
			, Down
			, Numpad0
			, Numpad1
			, Numpad2
			, Numpad3
			, Numpad4
			, Numpad5
			, Numpad6
			, Numpad7
			, Numpad8
			, Numpad9
			, NumpadDecimal
			, NumpadDivide
			, NumpadMultiply
			, NumpadSubstract
			, NumpadAdd
			, NumpadEnter
			, NumpadEqual
			, F1
			, F2
			, F3
			, F4
			, F5
			, F6
			, F7
			, F8
			, F9
			, F10
			, F11
			, F12
			, F13
			, F14
			, F15
			, F16
			, F17
			, F18
			, F19
			, F20
			, F21
			, F22
			, F23
			, F24
			, F25
			, Pause
			, CapsLock
			, ScrollLock
			, NumLock
			, PrintScreen
			, count
		};

		mistd::enum_map<Key, SwitchInput> keys;

		void resetChangedStates()
		{
			for (auto& key : keys)
			{
				key.resetChangedState();
			}
		}
	};

	struct Gamepad
	{
		enum class Button
		{
			Unknown = -1,

			// Default | Xbox
			A = 0,
			B,
			X,
			Y,
			LB,
			RB,
			Back,
			Start,
			Guide,
			LS,
			RS,
			Up,
			Right,
			Down,
			Left,
			count,

			// PlayStation
			Cross = A,
			Circle = B,
			Square = X,
			Triangle = Y,
			L1 = LB,
			L3 = LS,
			R1 = RB,
			R3 = RS,
			Select = Back,

			// Verbose
			LeftBumper = LB,
			RightBumper = RB,
			LeftThumb = LS,
			RightThumb = RS,
			DpadUp = Up,
			DpadDown = Down,
			DpadLeft = Left,
			DpadRight = Right
		};

		enum class Axis
		{
			Unknown = -1,

			// Default | Xbox
			LX = 0,
			LY,
			RX,
			RY,
			LT,
			RT,
			count,

			// PlayStation
			L2 = LT,
			R2 = RT,

			// Verbose
			LeftX = LX,
			LeftY = LY,
			RightX = RX,
			RightY = RY
		};

		SwitchInput isConnected;
		std::string_view name;
		mistd::enum_map<Button, SwitchInput> buttons;
		mistd::enum_map<Axis, AxisInput> axes;

		void resetChangedStates()
		{
			isConnected.resetChangedState();

			for (auto& button : buttons)
			{
				button.resetChangedState();
			}

			for (auto& axis : axes)
			{
				axis.resetChangedState();
			}
		}
	};

	static constexpr int32_t k_maxGamepadCount = 16;

	// TODO: why is it in this namespace? seems like input and window belong to the same simpler module.
	struct Inputs
	{
		Mouse mouse;
		Keyboard keyboard;
		std::array<Gamepad, k_maxGamepadCount> gamepads;

		void resetChangedStates()
		{
			mouse.resetChangedStates();
			keyboard.resetChangedStates();
			for (auto& gamepad : gamepads)
			{
				gamepad.resetChangedStates();
			}
		}
	};
}
