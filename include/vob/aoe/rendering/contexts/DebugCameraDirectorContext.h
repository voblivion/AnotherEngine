#pragma once

#include "vob/aoe/input/GameInput.h"


namespace vob::aoegl
{
	struct DebugCameraDirectorContext
	{
		aoein::GameInputEventId prevCameraInputEventId = {};
		aoein::GameInputEventId nextCameraInputEventId = {};
	};
}
