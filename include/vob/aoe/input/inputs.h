#pragma once

#include <vob/misc/std/enum_map.h>

#include <string_view>


namespace vob::aoein
{
	/// <summary>
	/// A one/off type of input, such as a key on a keyboard, a mouse button, etc.
	/// </summary>
	class axis_input
	{
	public:
		float get_change() const
		{
			return m_change;
		}

		float get_value() const
		{
			return m_value;
		}

		void update_state(float const a_value)
		{
			m_change = a_value - m_value;
			m_value = a_value;
		}

		void reset_changed_state()
		{
			m_change = 0.0f;
		}

	private:
		float m_change = 0.0f;
		float m_value = -1.0f;
	};

	/// <summary>
	/// A linear type of input, such as a trigger on a controller or the mouse's x/y positions.
	/// </summary>
	struct switch_input
	{
	public:
		bool has_changed() const
		{
			return m_hasChanged;
		}

		bool is_pressed() const
		{
			return m_isPressed;
		}

		void update_state(bool const a_isPressed)
		{
			m_hasChanged = m_isPressed != a_isPressed;
			m_isPressed = a_isPressed;
		}

		void reset_changed_state()
		{
			m_hasChanged = false;
		}

		bool was_pressed() const
		{
			return has_changed() && is_pressed();
		}

	private:
		bool m_isPressed = false;
		bool m_hasChanged = false;
	};

	/// <summary>
	/// State of mouse inputs (hover, positions, buttons).
	/// </summary>
	struct mouse
	{
		enum class axis
		{
			unkown = -1,
			X = 0,
			Y = 1,
			count
		};

		enum class button
		{
			unknown = -1,
			M1 = 0,
			M2,
			M3,
			M4,
			M5,
			M6,
			M7,
			M8,
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

		switch_input hover;
		mistd::enum_map<axis, axis_input> axes;
		mistd::enum_map<button, switch_input> buttons;

		void reset_changed_states()
		{
			hover.reset_changed_state();

			for (auto& axis : axes)
			{
				axis.reset_changed_state();
			}

			for (auto& button : buttons)
			{
				button.reset_changed_state();
			}

			// Scroll wheel only provides changes (could be an axis but not great)
			buttons[button::ScrollUp].update_state(false);
			buttons[button::ScrollDown].update_state(false);
		}
	};

	/// <summary>
	/// State of keyboard inputs (keys).
	/// </summary>
	struct keyboard
	{
		enum class key
		{
			unknown = -1
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

		mistd::enum_map<key, switch_input> keys;

		void reset_changed_states()
		{
			for (auto& key : keys)
			{
				key.reset_changed_state();
			}
		}
	};

	/// <summary>
	/// State of a gamepad inputs (is_connected, name, buttons, axes).
	/// </summary>
	struct gamepad
	{
		enum class button
		{
			unknown = -1,

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

			// Playstation
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

		enum class axis
		{
			unknown = -1,

			// Default | Xbox
			LX = 0,
			LY,
			RX,
			RY,
			LT,
			RT,
			count,

			// Playstation
			L2 = LT,
			R2 = RT,

			// Verbose
			LeftX = LX,
			LeftY = LY,
			RightX = RX,
			RightY = RY
		};

		switch_input is_connected;
		std::string_view name;
		mistd::enum_map<button, switch_input> buttons;
		mistd::enum_map<axis, axis_input> axes;

		void reset_changed_states()
		{
			is_connected.reset_changed_state();

			for (auto& button : buttons)
			{
				button.reset_changed_state();
			}

			for (auto& axis : axes)
			{
				axis.reset_changed_state();
			}
		}
	};

	using gamepad_index = size_t;

	constexpr gamepad_index k_maxGamepadCount = 16;

	/// <summary>
	/// State of a computer inputs (mouse, keyboard, gamepad(s)).
	/// </summary>
	struct inputs
	{
		mouse mouse;
		keyboard keyboard;
		std::array<gamepad, k_maxGamepadCount> gamepads;

		void reset_changed_states()
		{
			mouse.reset_changed_states();
			keyboard.reset_changed_states();
			for (auto& gamepad : gamepads)
			{
				gamepad.reset_changed_states();
			}
		}
	};
}
