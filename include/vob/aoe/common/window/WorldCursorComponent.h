#pragma once
#include <vob/aoe/core/ecs/Component.h>

#include <vob/aoe/common/render/IWindow.h>


namespace vob::aoe::common
{
	struct WorldCursorComponent final
		: public ecs::AComponent
	{
		common::CursorState m_state = common::CursorState::Normal;
	};
}
