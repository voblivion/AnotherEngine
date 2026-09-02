#pragma once

#include "vob/aoe/rendering/resources/GlDeleteQueue.h"

#include <functional>


namespace vob::aoegl
{
	struct GlDeleteQueueContext
	{
		std::reference_wrapper<GlDeleteQueue> deleteQueue;
	};
}
