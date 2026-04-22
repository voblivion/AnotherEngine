#pragma once

#include "vob/aoe/input/GameInput.h"


namespace vob::aoegl
{
	struct DebugCameraDirectorContext
	{
		aoein::GameInputEventId prevCameraInputEventId = {};
		aoein::GameInputEventId nextCameraInputEventId = {};
		// TODO: laziness, put it here for now
		aoein::GameInputEventId quitInputEventId = {};
	};
}
