#pragma once

#include <vob/aoe/core/ecs/Component.h>

#include <vob/aoe/common/render/IWindow.h>
#include <vob/aoe/common/input/raw/Keyboard.h>
#include <vob/aoe/common/input/raw/Mouse.h>

namespace vob::aoe::common
{
	struct WorldInputComponent final
		: public ecs::AComponent
	{
		Keyboard m_keyboard;
		Mouse m_mouse;
	};
}
