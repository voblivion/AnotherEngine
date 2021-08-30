#pragma once

#include <vob/aoe/core/ecs/Component.h>

#include <vob/aoe/common/render/IWindow.h>
#include <vob/aoe/common/window/Keyboard.h>
#include <vob/aoe/common/window/Mouse.h>

namespace vob::aoe::common
{
	struct InputComponent final
		: public ecs::AComponent
	{
		Keyboard m_keyboard;
		Mouse m_mouse;
	};
}
