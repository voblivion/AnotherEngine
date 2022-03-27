#pragma once

#include <vob/aoe/input/physical_inputs.h>
#include <vob/aoe/input/gamepad.h>


namespace vob::aoein
{
	struct physical_axis_reference
	{
		float get_value(physical_inputs const& a_physicalInput)
		{
			if (m_gamepadAxis == gamepad::axis::unknown)
			{
				return -1.0f;
			}

			return a_physicalInput.m_gamepads[m_gamepadIndex].m_axes[m_gamepadAxis].get_value();
		}

		std::uint8_t m_gamepadIndex;
		gamepad::axis m_gamepadAxis = gamepad::axis::unknown;
	};
}
