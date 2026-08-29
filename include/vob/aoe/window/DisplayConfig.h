#pragma once

#include "glm/glm.hpp"

#include <cstdint>
#include <string>


namespace vob::aoewi
{
	struct DisplayConfig
	{
		enum class WindowMode
		{
			Windowed,
			Borderless,
			FullScreen
		};

		std::string monitorName;
		int32_t monitorIndex = 0;
		WindowMode windowMode = WindowMode::Borderless;
		glm::ivec2 windowSize = glm::ivec2{ 2560, 1440 };
		glm::ivec2 windowPosition = glm::ivec2{ 0, 0 };
		bool vsync = false;
	};
}
