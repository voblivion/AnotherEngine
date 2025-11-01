#pragma once

#include <cstdint>


namespace vob::aoeph
{
	struct CollisionContext
	{
		// Configuration
		int32_t updateStepCount = 10;

		// Runtime
		int32_t lastFixedTickIndexProcessed = -1;
	};
}
