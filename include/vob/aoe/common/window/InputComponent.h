#pragma once

#include <vob/aoe/common/window/Keyboard.h>
#include <vob/aoe/common/window/Mouse.h>
#include <vob/aoe/core/ecs/Component.h>

namespace vob::aoe::common
{
	struct InputComponent final
		: public ecs::AComponent
	{
		Keyboard m_keyboard;
		Mouse m_mouse;
	};
}
