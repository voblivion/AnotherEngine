#pragma once

#include <aoe/core/ecs/Component.h>
#include <aoe/time/Chrono.h>

namespace aoe
{
	namespace time
	{
		struct TimeComponent final
			: public ecs::ComponentDefaultImpl<TimeComponent>
		{
			// Attributes
			TimePoint m_frameStartTime;
			Duration m_frameDuration{ 0.0f };
			float m_elapsedTime = 0.0f;
		};
	}
}
