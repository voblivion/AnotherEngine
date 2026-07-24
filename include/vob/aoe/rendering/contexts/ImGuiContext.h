#pragma once

#include "vob/aoe/input/GameInput.h"


namespace vob::aoegl
{
	struct ImGuiContext
	{
		bool isDisplayed = false;
		aoein::GameInputEventId toggleDisplayEventId = {};
	};
}
