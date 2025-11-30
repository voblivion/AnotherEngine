#pragma once

#include <vob/aoe/input/InputBindings.h>
#include <vob/aoe/input/GameInput.h>


namespace vob::aoedb
{
	struct GhostControllerComponent
	{
		float moveSpeed = 10.0f;

		aoein::GameInputValueId lateralMoveValueId = 0;
		aoein::GameInputValueId longitudinalMoveValueId = 0;
		aoein::GameInputValueId verticalMoveValueId = 0;
		aoein::GameInputValueId pitchValueId = 0;
		aoein::GameInputValueId yawValueId = 0;
		aoein::GameInputValueId enableRotationValueId = 0;

		aoein::GameInputEventId decreaseSpeedEventId = 0;
		aoein::GameInputEventId increaseSpeedEventId = 0;
	};
}
