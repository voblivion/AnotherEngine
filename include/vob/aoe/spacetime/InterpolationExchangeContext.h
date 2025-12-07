#pragma once

#include <chrono>


namespace vob::aoest
{
	struct InterpolationExchangeContext
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> targetTime = std::chrono::high_resolution_clock::now();
	};
}
