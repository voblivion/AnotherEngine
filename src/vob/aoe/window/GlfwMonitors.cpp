#include "vob/aoe/window/GlfwMonitors.h"

#include "vob/aoe/debug/Check.h"

#include "GLFW/glfw3.h"

#include <algorithm>
#include <format>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "windows.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#endif

namespace vob::aoewi
{
	namespace
	{
		GLFWmonitor* getMonitorHandle(int32_t const a_index)
		{
			auto monitorCount = 0;
			auto** const monitors = glfwGetMonitors(&monitorCount);
			if (monitors == nullptr || a_index < 0 || a_index >= monitorCount)
			{
				return nullptr;
			}
			return monitors[a_index];
		}

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

		bool findTargetDeviceName(GLFWmonitor* const a_monitor, DISPLAYCONFIG_TARGET_DEVICE_NAME& o_targetName)
		{
			auto const* const adapterName = glfwGetWin32Adapter(a_monitor);
			if (adapterName == nullptr)
			{
				return false;
			}

			auto pathCount = UINT32{};
			auto modeCount = UINT32{};
			if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount) != ERROR_SUCCESS)
			{
				return false;
			}

			auto paths = std::vector<DISPLAYCONFIG_PATH_INFO>(pathCount);
			auto modes = std::vector<DISPLAYCONFIG_MODE_INFO>(modeCount);
			if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathCount, paths.data(), &modeCount, modes.data(), nullptr)
				!= ERROR_SUCCESS)
			{
				return false;
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

				o_targetName = DISPLAYCONFIG_TARGET_DEVICE_NAME{};
				o_targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
				o_targetName.header.size = sizeof(o_targetName);
				o_targetName.header.adapterId = path.targetInfo.adapterId;
				o_targetName.header.id = path.targetInfo.id;
				if (DisplayConfigGetDeviceInfo(&o_targetName.header) != ERROR_SUCCESS)
				{
					continue;
				}
				return true;
			}
			return false;
		}

		std::string getFriendlyMonitorName(GLFWmonitor* const a_monitor)
		{
			auto targetName = DISPLAYCONFIG_TARGET_DEVICE_NAME{};
			return findTargetDeviceName(a_monitor, targetName) ? fromWideString(targetName.monitorFriendlyDeviceName)
															   : std::string{};
		}

		std::string getDeviceInstancePath(GLFWmonitor* const a_monitor)
		{
			auto targetName = DISPLAYCONFIG_TARGET_DEVICE_NAME{};
			if (!findTargetDeviceName(a_monitor, targetName))
			{
				return {};
			}

			auto const devicePath = fromWideString(targetName.monitorDevicePath);
			auto const start = devicePath.find("DISPLAY#");
			auto const end = devicePath.rfind('#');
			if (start == std::string::npos || end <= start)
			{
				return {};
			}

			return devicePath.substr(start, end - start);
		}
#endif

	}

	int32_t GlfwMonitors::getCount() const
	{
		auto monitorCount = 0;
		glfwGetMonitors(&monitorCount);
		return monitorCount;
	}

	std::optional<int32_t> GlfwMonitors::getPrimaryIndex() const
	{
		auto monitorCount = 0;
		auto** const monitors = glfwGetMonitors(&monitorCount);
		auto* const primaryMonitor = glfwGetPrimaryMonitor();
		if (monitors == nullptr || primaryMonitor == nullptr)
		{
			return std::nullopt;
		}

		for (auto i = 0; i < monitorCount; ++i)
		{
			if (monitors[i] == primaryMonitor)
			{
				return i;
			}
		}
		return std::nullopt;
	}

	std::optional<int32_t> GlfwMonitors::getIndex(std::string_view const a_id) const
	{
		if (a_id.empty())
		{
			return std::nullopt;
		}

		auto const monitorCount = getCount();
		for (auto monitorIndex = 0; monitorIndex < monitorCount; ++monitorIndex)
		{
			if (getId(monitorIndex) == a_id)
			{
				return monitorIndex;
			}
		}
		return std::nullopt;
	}

	std::optional<int32_t> GlfwMonitors::getIndexAt(glm::ivec2 const a_position) const
	{
		auto const monitorCount = getCount();
		for (auto monitorIndex = 0; monitorIndex < monitorCount; ++monitorIndex)
		{
			auto const origin = getPosition(monitorIndex);
			auto const size = getResolution(monitorIndex);
			if (a_position.x >= origin.x && a_position.x < origin.x + size.x && a_position.y >= origin.y
				&& a_position.y < origin.y + size.y)
			{
				return monitorIndex;
			}
		}
		return std::nullopt;
	}

	std::optional<int32_t> GlfwMonitors::resolveIndex(
		std::string_view const a_name, std::string_view const a_id, int32_t const a_index) const
	{
		auto const monitorCount = getCount();
		auto const isIndexValid = a_index >= 0 && a_index < monitorCount;

		if (!a_name.empty() && !a_id.empty())
		{
			for (auto monitorIndex = 0; monitorIndex < monitorCount; ++monitorIndex)
			{
				if (getName(monitorIndex) == a_name && getId(monitorIndex) == a_id)
				{
					return monitorIndex;
				}
			}
		}

		if (!a_name.empty() && isIndexValid && getName(a_index) == a_name)
		{
			return a_index;
		}

		if (!a_name.empty())
		{
			for (auto monitorIndex = 0; monitorIndex < monitorCount; ++monitorIndex)
			{
				if (getName(monitorIndex) == a_name)
				{
					return monitorIndex;
				}
			}
		}

		if (isIndexValid)
		{
			return a_index;
		}

		return getPrimaryIndex();
	}

	std::string GlfwMonitors::getId(int32_t const a_index) const
	{
		auto* const monitor = getMonitorHandle(a_index);
		if (monitor == nullptr)
		{
			return {};
		}

#if defined(_WIN32)
		auto deviceInstancePath = getDeviceInstancePath(monitor);
		VOB_AOE_CHECK_LOG(
			!deviceInstancePath.empty(), "Failed to identify monitor {}, falling back to its index.", a_index);
		return !deviceInstancePath.empty() ? std::move(deviceInstancePath) : std::format("{}", a_index);
#else
		VOB_AOE_CHECK_TERMINATE(false, "Monitor identification is not implemented on this platform.");
		return {};
#endif
	}

	std::string GlfwMonitors::getName(int32_t const a_index) const
	{
		auto* const monitor = getMonitorHandle(a_index);
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

	glm::ivec2 GlfwMonitors::getResolution(int32_t const a_index) const
	{
		auto* const monitor = getMonitorHandle(a_index);
		if (monitor == nullptr)
		{
			return {};
		}

		auto const* const videoMode = glfwGetVideoMode(monitor);
		return videoMode != nullptr ? glm::ivec2{ videoMode->width, videoMode->height } : glm::ivec2{};
	}

	glm::ivec2 GlfwMonitors::getPosition(int32_t const a_index) const
	{
		auto* const monitor = getMonitorHandle(a_index);
		if (monitor == nullptr)
		{
			return {};
		}

		auto position = glm::ivec2{};
		glfwGetMonitorPos(monitor, &position.x, &position.y);
		return position;
	}

	std::vector<glm::ivec2> GlfwMonitors::getSupportedResolutions(int32_t const a_index) const
	{
		auto* const monitor = getMonitorHandle(a_index);
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
