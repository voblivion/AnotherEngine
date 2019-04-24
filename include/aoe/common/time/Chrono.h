#pragma once

#include <chrono>

namespace aoe
{
	namespace common
	{
		using Clock = std::chrono::high_resolution_clock;
		using TimePoint = std::chrono::time_point<Clock>;
		using Duration = std::chrono::duration<float>;
	}
}