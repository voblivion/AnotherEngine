#include <vob/aoe/rendering/ImGuiUtils.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"


namespace vob::aoegl
{
	void initializeImGui(aoewi::GlfwWindow const& a_glfwWindow)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImPlot::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		ImGui_ImplGlfw_InitForOpenGL(a_glfwWindow.getNativeHandle(), false);
		ImGui_ImplOpenGL3_Init();
	}

	void terminateImGui()
	{
		ImPlot::DestroyContext();
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
}
