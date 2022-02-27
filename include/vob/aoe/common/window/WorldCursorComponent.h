#pragma once

#include <vob/aoe/common/render/IWindow.h>


namespace vob::aoe::common
{
	struct WorldCursorComponent final
	{
		common::CursorState m_state = common::CursorState::Normal;
	};
}
