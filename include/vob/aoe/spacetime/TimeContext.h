#pragma once

#include <chrono>


namespace vob::aoest
{
	struct TimeContext
	{
		// Runtime
		std::chrono::time_point<std::chrono::high_resolution_clock> tickStartTime = std::chrono::high_resolution_clock::now();
		std::chrono::nanoseconds elapsedTime;
	};
}
