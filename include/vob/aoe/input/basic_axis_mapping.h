#pragma once

#include <vob/misc/physics/measure.h>


namespace vob::aoein
{
	struct physical_inputs;

	class basic_axis_mapping
	{
	public:
		// ACCESSROS
		virtual float get_value() const = 0;

		// MANIPULATORS
		virtual void update(physical_inputs const&, misph::measure_time) = 0;
	};
}
