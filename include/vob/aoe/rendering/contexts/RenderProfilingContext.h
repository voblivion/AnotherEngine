#pragma once

#include "vob/aoe/rendering/GpuTimer.h"
#include "vob/aoe/rendering/GraphicTypes.h"

#include <array>
#include <cstdint>
#include <vector>


namespace vob::aoegl
{
	struct RenderProfilingContext
	{
		std::vector<GpuTimer> timers;
		int32_t readTimerSlotIndex = -1;
		int32_t writeTimerSlotIndex = 0;

		int32_t accumulationIndex = 0;
		int32_t accumulationCount = 50;
		int32_t runningDroppedFrameCount = 0;
		int32_t lastDroppedFrameCount = 0;

		int32_t lightCount = 0;
		int32_t staticOpaqueMeshCount = 0;
		int32_t riggedOpaqueMeshCount = 0;
	};
}
