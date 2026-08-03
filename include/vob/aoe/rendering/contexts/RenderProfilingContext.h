#pragma once

#include "vob/aoe/rendering/GpuProfiler.h"

#include <cstdint>


namespace vob::aoegl
{
	struct RenderProfilingContext
	{
		GpuProfiler gpuProfiler;

		int32_t lightCount = 0;
		int32_t staticOpaqueMeshCount = 0;
		int32_t riggedOpaqueMeshCount = 0;
	};
}
