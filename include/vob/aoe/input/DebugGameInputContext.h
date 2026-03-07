#pragma once

#include <vob/aoe/input/GameInput.h>

#include <cstdint>
#include <string_view>
#include <vector>


namespace vob::aoein
{
	struct DebugGameInputContext
	{
		struct DebugValue
		{
			char const* name = nullptr;
			GameInputValueId id = {};
			std::vector<float> values;
		};

		struct DebugEvent
		{
			char const* name = nullptr;
			GameInputEventId id = {};
			std::vector<int32_t> events;
		};

		int32_t historyLength = 128;
		int32_t nextIndex = 0;
		std::vector<DebugValue> values;
		std::vector<DebugEvent> events;
	};
}
