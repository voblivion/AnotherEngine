#include "vob/aoe/window/GlfwWindowUtils.h"

#include <algorithm>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif


namespace vob::aoewi
{
	namespace
	{
#if defined(_WIN32)
		std::string fromWideString(wchar_t const* const a_text)
		{
			if (a_text == nullptr || a_text[0] == L'\0')
			{
				return {};
			}

			auto const size = WideCharToMultiByte(CP_UTF8, 0, a_text, -1, nullptr, 0, nullptr, nullptr);
			if (size <= 1)
			{
				return {};
			}

			auto result = std::string(static_cast<std::size_t>(size) - 1, '\0');
			WideCharToMultiByte(CP_UTF8, 0, a_text, -1, result.data(), size, nullptr, nullptr);
			return result;
		}

		// GLFW reports the monitor driver's description, which is "Generic PnP Monitor" for
		// every panel using the inbox driver; the EDID name only comes from the display config API
		std::string getFriendlyMonitorName(GLFWmonitor* const a_monitor)
		{
			auto const* const adapterName = glfwGetWin32Adapter(a_monitor);
			if (adapterName == nullptr)
			{
				return {};
			}

			auto pathCount = UINT32{};
			auto modeCount = UINT32{};
			if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount) != ERROR_SUCCESS)
			{
				return {};
			}

			auto paths = std::vector<DISPLAYCONFIG_PATH_INFO>(pathCount);
			auto modes = std::vector<DISPLAYCONFIG_MODE_INFO>(modeCount);
			if (QueryDisplayConfig(
				QDC_ONLY_ACTIVE_PATHS,
				&pathCount,
				paths.data(),
				&modeCount,
				modes.data(),
				nullptr) != ERROR_SUCCESS)
			{
				return {};
			}
			paths.resize(pathCount);

			for (auto const& path : paths)
			{
				auto sourceName = DISPLAYCONFIG_SOURCE_DEVICE_NAME{};
				sourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
				sourceName.header.size = sizeof(sourceName);
				sourceName.header.adapterId = path.sourceInfo.adapterId;
				sourceName.header.id = path.sourceInfo.id;
				if (DisplayConfigGetDeviceInfo(&sourceName.header) != ERROR_SUCCESS
					|| fromWideString(sourceName.viewGdiDeviceName) != adapterName)
				{
					continue;
				}

				auto targetName = DISPLAYCONFIG_TARGET_DEVICE_NAME{};
				targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
				targetName.header.size = sizeof(targetName);
				targetName.header.adapterId = path.targetInfo.adapterId;
				targetName.header.id = path.targetInfo.id;
				if (DisplayConfigGetDeviceInfo(&targetName.header) != ERROR_SUCCESS)
				{
					continue;
				}

				return fromWideString(targetName.monitorFriendlyDeviceName);
			}
			return {};
		}
#endif

		GLFWmonitor* getMonitor(int32_t const a_monitorIndex)
		{
			auto monitorCount = 0;
			auto** const monitors = glfwGetMonitors(&monitorCount);
			if (monitors == nullptr || a_monitorIndex < 0 || a_monitorIndex >= monitorCount)
			{
				return nullptr;
			}
			return monitors[a_monitorIndex];
		}
	}

	GLFWmonitor* findMonitor(DisplayConfig const& a_config)
	{
		auto monitorCount = 0;
		auto** const monitors = glfwGetMonitors(&monitorCount);
		if (monitors == nullptr || monitorCount == 0)
		{
			return glfwGetPrimaryMonitor();
		}

		auto* firstNameMatch = static_cast<GLFWmonitor*>(nullptr);
		if (!a_config.monitorName.empty())
		{
			for (auto i = 0; i < monitorCount; ++i)
			{
				auto const* const name = glfwGetMonitorName(monitors[i]);
				if (name == nullptr || a_config.monitorName != name)
				{
					continue;
				}

				if (i == a_config.monitorIndex)
				{
					return monitors[i];
				}
				if (firstNameMatch == nullptr)
				{
					firstNameMatch = monitors[i];
				}
			}
		}

		if (firstNameMatch != nullptr)
		{
			return firstNameMatch;
		}

		if (a_config.monitorIndex >= 0 && a_config.monitorIndex < monitorCount)
		{
			return monitors[a_config.monitorIndex];
		}

		return glfwGetPrimaryMonitor();
	}

	int32_t getMonitorCount()
	{
		auto monitorCount = 0;
		glfwGetMonitors(&monitorCount);
		return monitorCount;
	}

	int32_t getPrimaryMonitorIndex()
	{
		auto monitorCount = 0;
		auto** const monitors = glfwGetMonitors(&monitorCount);
		auto* const primaryMonitor = glfwGetPrimaryMonitor();
		if (monitors == nullptr || primaryMonitor == nullptr)
		{
			return -1;
		}

		for (auto i = 0; i < monitorCount; ++i)
		{
			if (monitors[i] == primaryMonitor)
			{
				return i;
			}
		}
		return -1;
	}

	glm::ivec2 getMonitorResolution(int32_t const a_monitorIndex)
	{
		auto* const monitor = getMonitor(a_monitorIndex);
		if (monitor == nullptr)
		{
			return {};
		}

		auto const* const videoMode = glfwGetVideoMode(monitor);
		return videoMode != nullptr ? glm::ivec2{ videoMode->width, videoMode->height } : glm::ivec2{};
	}

	glm::ivec2 getMonitorPosition(int32_t const a_monitorIndex)
	{
		auto* const monitor = getMonitor(a_monitorIndex);
		if (monitor == nullptr)
		{
			return {};
		}

		auto position = glm::ivec2{};
		glfwGetMonitorPos(monitor, &position.x, &position.y);
		return position;
	}

	std::string getMonitorName(int32_t const a_monitorIndex)
	{
		auto* const monitor = getMonitor(a_monitorIndex);
		if (monitor == nullptr)
		{
			return {};
		}

#if defined(_WIN32)
		auto friendlyName = getFriendlyMonitorName(monitor);
		if (!friendlyName.empty())
		{
			return friendlyName;
		}
#endif

		auto const* const name = glfwGetMonitorName(monitor);
		return name != nullptr ? std::string{ name } : std::string{};
	}

	std::vector<glm::ivec2> getSupportedResolutions(int32_t const a_monitorIndex)
	{
		auto* const monitor = getMonitor(a_monitorIndex);
		if (monitor == nullptr)
		{
			return {};
		}

		auto videoModeCount = 0;
		auto const* const videoModes = glfwGetVideoModes(monitor, &videoModeCount);
		if (videoModes == nullptr)
		{
			return {};
		}

		auto resolutions = std::vector<glm::ivec2>{};
		resolutions.reserve(videoModeCount);
		for (auto i = 0; i < videoModeCount; ++i)
		{
			auto const resolution = glm::ivec2{ videoModes[i].width, videoModes[i].height };
			if (std::ranges::find(resolutions, resolution) == resolutions.end())
			{
				resolutions.push_back(resolution);
			}
		}
		std::ranges::reverse(resolutions);
		return resolutions;
	}
}
