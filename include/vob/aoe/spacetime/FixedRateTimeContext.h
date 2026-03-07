#pragma once

#include <chrono>

namespace vob::aoest
{
	struct FixedRateTimeContext
	{
		// Configuration
		std::chrono::nanoseconds tickDuration = std::chrono::milliseconds(10);

		// Runtime
		int32_t debugRemainingTickCount = -1;
		std::chrono::time_point<std::chrono::high_resolution_clock> tickStartTime = std::chrono::high_resolution_clock::now();
		int32_t tickIndex = 0;
	};
}
