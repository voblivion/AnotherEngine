#pragma once

#include <chrono>


namespace vob::aoest
{
	struct InterpolationTimeComponent
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> sourceTime = std::chrono::high_resolution_clock::now();
		std::chrono::time_point<std::chrono::high_resolution_clock> targetTime = std::chrono::high_resolution_clock::now();
	};
}
