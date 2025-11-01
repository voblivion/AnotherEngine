#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/window/GlfwWindow.h>


namespace vob::aoegl
{
	VOB_AOE_API void initializeImGui(aoewi::GlfwWindow const& a_glfwWindow);
	VOB_AOE_API void terminateImGui();
}
