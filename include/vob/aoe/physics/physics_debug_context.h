#pragma once

#include <vob/misc/physics/measure_literals.h>


namespace vob::aoeph
{
	struct physics_debug_context
	{
		bool is_dynamic_shape_debug_enabled = true;
		bool is_dynamic_contact_debug_enabled = true;
		bool is_dynamic_velocity_debug_enabled = false;
		bool is_aabb_debug_enabled = false;
		bool is_static_shape_debug_enabled = true;
	};
}
