#pragma once

#include <vob/aoe/input/physical_inputs.h>
#include <vob/aoe/input/mouse.h>


namespace vob::aoein
{
	struct mouse_move_axis_reference
	{
		float get_value(physical_inputs const& a_physicalInput)
		{
			if (m_mouseAxis == mouse::axis::unkown)
			{
				return 0.0f;
			}

			return a_physicalInput.m_mouse.m_move[static_cast<std::size_t>(m_mouseAxis)];
		}

		mouse::axis m_mouseAxis = mouse::axis::unkown;
	};
}
