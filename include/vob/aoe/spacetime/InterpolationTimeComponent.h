#pragma once

#include <array>
#include <chrono>


namespace vob::aoest
{
	struct InterpolationTimeComponent
	{
		std::array<std::chrono::time_point<std::chrono::high_resolution_clock>, 4> times;
		int32_t endIndex = 0;

		std::chrono::time_point<std::chrono::high_resolution_clock> sourceTime = std::chrono::high_resolution_clock::now();
		std::chrono::time_point<std::chrono::high_resolution_clock> targetTime = std::chrono::high_resolution_clock::now();
	};
}
