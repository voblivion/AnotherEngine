#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"

#include <cstdint>
#include <vector>


namespace vob::aoegl
{
	struct LimitFramesInFlightContext
	{
		int32_t frameInFlightCapacity = 2;
		std::vector<GLsync> fences;
		int32_t index = 0;
	};
}
