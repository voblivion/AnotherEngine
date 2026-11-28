#include <vob/aoe/rendering/ImGuiUtils.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#include <iterator>
#include <string>
#include <string_view>


namespace vob::aoegl
{
	namespace
	{
		constexpr std::string_view k_iniName = "imgui.ini";
	}

	void initializeImGui(aoewi::GlfwWindow const& a_glfwWindow, aoesg::IStorage const& a_storage)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImPlot::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.IniFilename = nullptr;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		if (auto const stream = a_storage.openForRead(aoesg::StorageDomain::Settings, k_iniName))
		{
			auto const ini = std::string{ std::istreambuf_iterator<char>{ *stream }, std::istreambuf_iterator<char>{} };
			ImGui::LoadIniSettingsFromMemory(ini.data(), ini.size());
		}
		ImGui_ImplGlfw_InitForOpenGL(a_glfwWindow.getNativeHandle(), false);
		ImGui_ImplOpenGL3_Init();
	}

	void terminateImGui(aoesg::IStorage& a_storage)
	{
		if (auto const stream = a_storage.openForWrite(aoesg::StorageDomain::Settings, k_iniName))
		{
			auto iniSize = std::size_t{};
			auto const* const ini = ImGui::SaveIniSettingsToMemory(&iniSize);
			stream->write(ini, static_cast<std::streamsize>(iniSize));
		}
		ImPlot::DestroyContext();
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
}
