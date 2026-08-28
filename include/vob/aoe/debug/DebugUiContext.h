#pragma once

#include "vob/aoe/input/GameInput.h"


namespace vob::aoedb
{
	struct DebugUiContext
	{
		bool isDisplayed = false;
		aoein::GameInputEventId toggleDisplayEventId = {};
	};
}
