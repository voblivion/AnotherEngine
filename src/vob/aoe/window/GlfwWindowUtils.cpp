#include "vob/aoe/window/GlfwWindowUtils.h"


namespace vob::aoewi
{
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
}
