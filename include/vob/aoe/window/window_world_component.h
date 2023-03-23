#pragma once

#include <vob/aoe/window/window_interface.h>

#include <functional>


namespace vob::aoewi
{
	struct window_world_component
	{
		std::reference_wrapper<window_interface> m_window;
	};
}
