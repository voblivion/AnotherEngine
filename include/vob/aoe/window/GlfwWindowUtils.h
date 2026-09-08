#pragma once

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>


namespace vob::aoewi
{
	GLFWmonitor* findMonitor(std::string_view a_monitorName, int32_t a_monitorIndex);

	int32_t getMonitorCount();

	int32_t getPrimaryMonitorIndex();

	std::string getMonitorName(int32_t a_monitorIndex);

	glm::ivec2 getMonitorResolution(int32_t a_monitorIndex);

	glm::ivec2 getMonitorPosition(int32_t a_monitorIndex);

	std::vector<glm::ivec2> getSupportedResolutions(int32_t a_monitorIndex);
}
