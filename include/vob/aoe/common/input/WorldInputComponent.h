#pragma once

#include <vob/aoe/common/input/Gamepad.h>
#include <vob/aoe/common/input/Keyboard.h>
#include <vob/aoe/common/input/Mouse.h>

#include <array>


namespace vob::aoe::common
{
	constexpr std::size_t c_maxGamepadCount = 16u;

	struct WorldInputComponent final
	{
		Keyboard m_keyboard;
		Mouse m_mouse;
		std::array<Gamepad, c_maxGamepadCount> m_gamepads;
	};
}
