#pragma once

#include "vob/aoe/rendering/resources/GpuDeleteQueue.h"

#include <functional>


namespace vob::aoegl
{
	struct GpuDeleteQueueContext
	{
		std::reference_wrapper<GpuDeleteQueue> deleteQueue;
	};
}
