#pragma once

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>


namespace vob::aoewi
{
	GLFWmonitor* resolveMonitor(
		std::string_view a_monitorName, std::string_view a_monitorId, int32_t a_monitorIndex);

	int32_t findMonitorIndex(
		std::string_view a_monitorName, std::string_view a_monitorId, int32_t a_monitorIndex);

	GLFWmonitor* getMonitor(int32_t a_monitorIndex);

	std::string getMonitorId(int32_t a_monitorIndex);

	int32_t getMonitorCount();

	int32_t getPrimaryMonitorIndex();

	std::string getMonitorName(int32_t a_monitorIndex);

	glm::ivec2 getMonitorResolution(int32_t a_monitorIndex);

	glm::ivec2 getMonitorPosition(int32_t a_monitorIndex);

	int32_t getMonitorIndexAt(glm::ivec2 a_position);

	std::vector<glm::ivec2> getSupportedResolutions(int32_t a_monitorIndex);
}
