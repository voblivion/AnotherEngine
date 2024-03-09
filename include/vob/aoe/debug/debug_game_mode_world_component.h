#pragma once

#include <vob/aoe/engine/world_data_provider.h>
#include <vob/aoe/input/bindings.h>

#include <vector>


namespace vob::aoedb
{
	struct debug_game_mode_world_component
	{
		aoein::bindings::switch_id m_switchActiveController = 0;
		aoein::bindings::switch_id m_switchActiveCamera = 0;
	};
}
