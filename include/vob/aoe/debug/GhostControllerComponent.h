#pragma once

#include <vob/aoe/input/InputBindings.h>


namespace vob::aoedb
{
	struct GhostControllerComponent
	{
		float moveSpeed = 10.0f;

		aoein::InputBindings::AxisId lateralMoveBinding = 0;
		aoein::InputBindings::AxisId longitudinalMoveBinding = 0;
		aoein::InputBindings::AxisId verticalMoveMapping = 0;
		aoein::InputBindings::AxisId pitchBinding = 0;
		aoein::InputBindings::AxisId yawBinding = 0;
		aoein::InputBindings::SwitchId enableRotationBinding = 0;
		aoein::InputBindings::SwitchId decreaseSpeedBinding = 0;
		aoein::InputBindings::SwitchId increaseSpeedBinding = 0;
	};
}
