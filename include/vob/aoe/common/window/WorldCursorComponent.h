#pragma once

#include <vob/aoe/common/_render/IWindow.h>


namespace vob::aoe::common
{
	struct WorldCursorComponent final
	{
		common::CursorState m_state = common::CursorState::Normal;
	};
}
