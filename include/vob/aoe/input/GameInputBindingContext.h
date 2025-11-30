#pragma once

#include <vob/aoe/input/GameInput.h>
#include <vob/aoe/input/InputBinding.h>

#include <cinttypes>


namespace vob::aoein
{
	struct GameInputBindingContext
	{
		std::vector<std::pair<GameInputValueId, std::shared_ptr<AInputValueBinding>>> values;
		std::vector<std::pair<GameInputEventId, std::shared_ptr<AInputEventBinding>>> events;
	};
}
