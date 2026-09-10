#include "vob/aoe/window/GlfwWindowUtils.h"

#include <algorithm>
#include <array>
#include <format>

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

		bool findTargetDeviceName(
			GLFWmonitor* const a_monitor, DISPLAYCONFIG_TARGET_DEVICE_NAME& o_targetName)
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
			if (QueryDisplayConfig(
				QDC_ONLY_ACTIVE_PATHS,
				&pathCount,
				paths.data(),
				&modeCount,
				modes.data(),
				nullptr) != ERROR_SUCCESS)
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
			return findTargetDeviceName(a_monitor, targetName)
				? fromWideString(targetName.monitorFriendlyDeviceName)
				: std::string{};
		}

		std::string readEdidSerial(std::wstring_view const a_devicePath)
		{
			auto key = std::wstring{ L"SYSTEM\\CurrentControlSet\\Enum\\" };
			auto const start = a_devicePath.find(L"DISPLAY#");
			auto const end = a_devicePath.rfind(L'#');
			if (start == std::wstring_view::npos || end <= start)
			{
				return {};
			}

			key += a_devicePath.substr(start, end - start);
			std::ranges::replace(key, L'#', L'\\');
			key += L"\\Device Parameters";

			auto edid = std::array<BYTE, 256>{};
			auto edidSize = DWORD{ edid.size() };
			auto const result = RegGetValueW(
				HKEY_LOCAL_MACHINE,
				key.c_str(),
				L"EDID",
				RRF_RT_REG_BINARY,
				nullptr,
				edid.data(),
				&edidSize);
			if (result != ERROR_SUCCESS || edidSize < 128)
			{
				return {};
			}

			for (auto block = std::size_t{ 54 }; block + 18 <= 126; block += 18)
			{
				if (edid[block] == 0 && edid[block + 1] == 0 && edid[block + 3] == 0xFF)
				{
					auto serial = std::string{};
					for (auto i = block + 5; i < block + 18 && edid[i] != 0x0A; ++i)
					{
						serial += static_cast<char>(edid[i]);
					}
					while (!serial.empty() && serial.back() == ' ')
					{
						serial.pop_back();
					}
					if (!serial.empty())
					{
						return serial;
					}
				}
			}

			auto const numericSerial = static_cast<std::uint32_t>(edid[12])
				| (static_cast<std::uint32_t>(edid[13]) << 8)
				| (static_cast<std::uint32_t>(edid[14]) << 16)
				| (static_cast<std::uint32_t>(edid[15]) << 24);
			return numericSerial != 0 ? std::to_string(numericSerial) : std::string{};
		}

		std::string getEdidMonitorId(GLFWmonitor* const a_monitor)
		{
			auto targetName = DISPLAYCONFIG_TARGET_DEVICE_NAME{};
			if (!findTargetDeviceName(a_monitor, targetName))
			{
				return {};
			}

			auto const manufactureId = _byteswap_ushort(targetName.edidManufactureId);
			auto const manufacturer = std::string{
				static_cast<char>('A' + ((manufactureId >> 10) & 0x1F) - 1),
				static_cast<char>('A' + ((manufactureId >> 5) & 0x1F) - 1),
				static_cast<char>('A' + (manufactureId & 0x1F) - 1) };

			auto const serial = readEdidSerial(targetName.monitorDevicePath);
			return serial.empty()
				? std::string{}
				: std::format("{}-{:04X}-{}", manufacturer, targetName.edidProductCodeId, serial);
		}
#endif

	}

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

	std::string getMonitorId(int32_t const a_monitorIndex)
	{
		auto* const monitor = getMonitor(a_monitorIndex);
		if (monitor == nullptr)
		{
			return {};
		}

#if defined(_WIN32)
		return getEdidMonitorId(monitor);
#else
		return {};
#endif
	}

	int32_t findMonitorIndex(
		std::string_view const a_monitorName, std::string_view const a_monitorId, int32_t const a_monitorIndex)
	{
		auto const monitorCount = getMonitorCount();
		if (monitorCount == 0)
		{
			return -1;
		}

		if (!a_monitorName.empty() && !a_monitorId.empty())
		{
			for (auto monitorIndex = 0; monitorIndex < monitorCount; ++monitorIndex)
			{
				if (getMonitorName(monitorIndex) == a_monitorName
					&& getMonitorId(monitorIndex) == a_monitorId)
				{
					return monitorIndex;
				}
			}
		}

		if (!a_monitorName.empty()
			&& a_monitorIndex >= 0 && a_monitorIndex < monitorCount
			&& getMonitorName(a_monitorIndex) == a_monitorName)
		{
			return a_monitorIndex;
		}

		if (!a_monitorName.empty())
		{
			for (auto monitorIndex = 0; monitorIndex < monitorCount; ++monitorIndex)
			{
				if (getMonitorName(monitorIndex) == a_monitorName)
				{
					return monitorIndex;
				}
			}
		}

		if (a_monitorIndex >= 0 && a_monitorIndex < monitorCount)
		{
			return a_monitorIndex;
		}

		return getPrimaryMonitorIndex();
	}

	GLFWmonitor* resolveMonitor(
		std::string_view const a_monitorName, std::string_view const a_monitorId, int32_t const a_monitorIndex)
	{
		auto* const monitor = getMonitor(findMonitorIndex(a_monitorName, a_monitorId, a_monitorIndex));
		return monitor != nullptr ? monitor : glfwGetPrimaryMonitor();
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

	int32_t getMonitorIndexAt(glm::ivec2 const a_position)
	{
		auto const monitorCount = getMonitorCount();
		for (auto monitorIndex = 0; monitorIndex < monitorCount; ++monitorIndex)
		{
			auto const origin = getMonitorPosition(monitorIndex);
			auto const size = getMonitorResolution(monitorIndex);
			if (a_position.x >= origin.x && a_position.x < origin.x + size.x
				&& a_position.y >= origin.y && a_position.y < origin.y + size.y)
			{
				return monitorIndex;
			}
		}
		return getPrimaryMonitorIndex();
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
