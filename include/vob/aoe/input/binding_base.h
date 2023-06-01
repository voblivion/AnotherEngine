#pragma once

#include <vob/aoe/input/inputs.h>

#include <vob/misc/physics/measure.h>


namespace vob::aoein
{
	struct input_binding_base
	{
		virtual void update(
			inputs const& a_inputs,
			misph::measure_time a_elapsedTime) = 0;
	};

	struct axis_binding_base : public input_binding_base
	{
		virtual float get_change() const = 0;
		virtual float get_value() const = 0;
	};

	struct switch_binding_base : public input_binding_base
	{
		virtual bool has_changed() const = 0;
		virtual bool is_pressed() const = 0;

		bool was_pressed() const
		{
			return has_changed() && is_pressed();
		}

		bool was_released() const
		{
			return has_changed() && !is_pressed();
		}
	};
}
