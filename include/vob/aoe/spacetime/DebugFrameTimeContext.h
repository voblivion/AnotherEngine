#pragma once

#include <chrono>


namespace vob::aoest
{
	struct DebugFrameTimeContext
	{
		std::chrono::nanoseconds frameTime;
	};
}
