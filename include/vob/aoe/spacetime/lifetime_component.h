#pragma once

#include <vob/misc/physics/measure.h>
#include <vob/misc/physics/measure_literals.h>

namespace vob::aoest
{
	using namespace misph::literals;

	struct lifetime_component
	{
		misph::measure_time m_remainingLifetime = 0.0_s;
	};
}
