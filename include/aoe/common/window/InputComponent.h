#pragma once

#include <aoe/common/window/Keyboard.h>
#include <aoe/common/window/Mouse.h>
#include <aoe/core/ecs/Component.h>

namespace aoe
{
	namespace common
	{
		struct InputComponent final
			: public ecs::AComponent
		{
			Keyboard m_keyboard;
			Mouse m_mouse;
		};
	}
}
