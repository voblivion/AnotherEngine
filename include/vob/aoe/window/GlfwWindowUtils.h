#pragma once

#include "vob/aoe/window/DisplayConfig.h"

#include <GLFW/glfw3.h>


namespace vob::aoewi
{
	GLFWmonitor* findMonitor(DisplayConfig const& a_config);
}
