#pragma once
#include <aoe/core/ecs/Component.h>


namespace aoe
{
	namespace common
	{
		struct CursorComponent final
			: public ecs::ComponentDefaultImpl<CursorComponent>
		{
			bool m_center = false;
			bool m_visible = true;
		};
	}
}
