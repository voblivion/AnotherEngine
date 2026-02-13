#pragma once

#include <chrono>


namespace vob::aoest
{
	struct InterpolationContext
	{
		std::chrono::nanoseconds offset = std::chrono::milliseconds(20);
	};
}
