#pragma once

#include "vob/aoe/window/DisplayConfig.h"

#include <GLFW/glfw3.h>

#include <string>
#include <vector>


namespace vob::aoewi
{
	GLFWmonitor* findMonitor(DisplayConfig const& a_config);

	int32_t getMonitorCount();

	int32_t getPrimaryMonitorIndex();

	std::string getMonitorName(int32_t a_monitorIndex);

	glm::ivec2 getMonitorResolution(int32_t a_monitorIndex);

	glm::ivec2 getMonitorPosition(int32_t a_monitorIndex);

	std::vector<glm::ivec2> getSupportedResolutions(int32_t a_monitorIndex);
}
