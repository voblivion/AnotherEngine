#pragma once

#include <vob/misc/physics/measure.h>


namespace vob::aoein
{
	struct physical_inputs;

	class basic_switch_mapping
	{
	public:
		// ACCESSORS
		virtual bool is_pressed() const = 0;
		virtual bool changed() const = 0;

		bool was_just_pressed() const
		{
			return changed() && is_pressed();
		}

		// MANIPULATORS
		virtual void update(physical_inputs const&, misph::measure_time) = 0;
	};
}