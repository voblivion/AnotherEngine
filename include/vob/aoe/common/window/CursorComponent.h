#pragma once
#include <vob/aoe/core/ecs/Component.h>


namespace vob::aoe::common
{
	struct CursorComponent final
		: public ecs::AComponent
	{
		common::CursorState m_state = common::CursorState::Normal;
	};
}
