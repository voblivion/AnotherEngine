#pragma once

#include <vob/aoe/input/switch_input.h>

#include <vob/misc/std/enum_map.h>


namespace vob::aoein
{
	class keyboard
	{
	public:
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

		mistd::enum_map<key, aoein::switch_input> m_keys{};

		void update()
		{
			for (auto& key : m_keys)
			{
				key.update();
			}
		}
	};
}
