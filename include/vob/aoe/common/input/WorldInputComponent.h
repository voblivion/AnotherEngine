#pragma once

#include <vob/aoe/common/render/IWindow.h>
#include <vob/aoe/common/input/physical/Gamepad.h>
#include <vob/aoe/common/input/physical/Keyboard.h>
#include <vob/aoe/common/input/physical/Mouse.h>

#include <vob/aoe/core/ecs/Component.h>

#include <array>


namespace vob::aoe::common
{
	struct WorldInputComponent final
		: public ecs::AComponent
	{
		Keyboard m_keyboard;
		Mouse m_mouse;
		std::array<Gamepad, 16> m_gamepads;
		// TODO: std::array<Joystick, 16> m_joysticks;
	};
}
