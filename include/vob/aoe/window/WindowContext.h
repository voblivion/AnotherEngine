#pragma once

#include <vob/aoe/window/Window.h>

#include <functional>


namespace vob::aoewi
{
	struct WindowContext
	{
		std::reference_wrapper<IWindow> window;
	};
}
