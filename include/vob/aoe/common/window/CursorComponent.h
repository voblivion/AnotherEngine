#pragma once
#include <vob/aoe/core/ecs/Component.h>


namespace vob::aoe::common
{
	struct CursorComponent final
		: public ecs::AComponent
	{
		bool m_center = false;
		bool m_visible = true;
	};
}
