#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/storage/IStorage.h>
#include <vob/aoe/window/GlfwWindow.h>


namespace vob::aoegl
{
	VOB_AOE_API void initializeImGui(aoewi::GlfwWindow const& a_glfwWindow, aoesg::IStorage const& a_storage);
	VOB_AOE_API void terminateImGui(aoesg::IStorage& a_storage);
}
