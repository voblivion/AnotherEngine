#pragma once

#include <aoe/core/ecs/Component.h>
#include <aoe/common/time/Chrono.h>

namespace aoe
{
	namespace common
	{
		struct TimeComponent final
			: ecs::AComponent
		{
			// Attributes
			TimePoint m_frameStartTime;
			Duration m_frameDuration{ 0.0f };
			float m_elapsedTime = 0.0f;
		};
	}
}
