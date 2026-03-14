#pragma once

#include <array>
#include <chrono>


namespace vob::aoest
{
	struct DebugSimulationFrameTimeEvent
	{
		float durationInMs;
	};

	struct DebugSimulationFrameTimeHistoryContext
	{
		int32_t historyLength = 512;
		int32_t nextIndex = 0;
		std::vector<float> durationsInMs;
	};
}
