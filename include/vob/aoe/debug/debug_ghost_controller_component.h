#pragma once

#include <vob/aoe/input/bindings.h>


namespace vob::aoedb
{
	struct debug_ghost_controller_component
	{
		float m_moveSpeed = 10.0f;

		aoein::bindings::axis_id m_lateralMoveMapping = 0;
		aoein::bindings::axis_id m_longitudinalMoveMapping = 0;
		aoein::bindings::axis_id m_verticalMoveMapping = 0;
		aoein::bindings::axis_id m_pitchMapping = 0;
		aoein::bindings::axis_id m_yawMapping = 0;
		aoein::bindings::switch_id m_enableViewMapping = 0;
		aoein::bindings::switch_id m_decreaseSpeedMapping = 0;
		aoein::bindings::switch_id m_increaseSpeedMapping = 0;
	};
}
