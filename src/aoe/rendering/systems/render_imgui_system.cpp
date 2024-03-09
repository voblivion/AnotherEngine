#include <vob/aoe/rendering/systems/render_imgui_system.h>

#include <vob/aoe/window/glfw_window.h>
#include <vob/aoe/window/window_input_world_component.h>
#include <vob/aoe/window/window_world_component.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


namespace
{
	void finalize_prev_frame()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void prepare_next_frame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	ImGuiKey to_imgui_key(vob::aoein::keyboard::key a_key)
	{
		constexpr std::array<ImGuiKey, std::to_underlying(vob::aoein::keyboard::key::count)> k_keyToImGuiKey{
			ImGuiKey_A
			, ImGuiKey_B
			, ImGuiKey_C
			, ImGuiKey_D
			, ImGuiKey_E
			, ImGuiKey_F
			, ImGuiKey_G
			, ImGuiKey_H
			, ImGuiKey_I
			, ImGuiKey_J
			, ImGuiKey_K
			, ImGuiKey_L
			, ImGuiKey_M
			, ImGuiKey_N
			, ImGuiKey_O
			, ImGuiKey_P
			, ImGuiKey_Q
			, ImGuiKey_R
			, ImGuiKey_S
			, ImGuiKey_T
			, ImGuiKey_U
			, ImGuiKey_V
			, ImGuiKey_W
			, ImGuiKey_X
			, ImGuiKey_Y
			, ImGuiKey_Z
			, ImGuiKey_0
			, ImGuiKey_1
			, ImGuiKey_2
			, ImGuiKey_3
			, ImGuiKey_4
			, ImGuiKey_5
			, ImGuiKey_6
			, ImGuiKey_7
			, ImGuiKey_8
			, ImGuiKey_9
			, ImGuiKey_Escape
			, ImGuiKey_LeftCtrl
			, ImGuiKey_LeftShift
			, ImGuiKey_LeftAlt
			, ImGuiKey_None // LSystem
			, ImGuiKey_RightCtrl
			, ImGuiKey_RightShift
			, ImGuiKey_RightAlt
			, ImGuiKey_None // RSystem
			, ImGuiKey_Menu
			, ImGuiKey_LeftBracket
			, ImGuiKey_RightBracket
			, ImGuiKey_GraveAccent
			, ImGuiKey_Semicolon
			, ImGuiKey_Comma
			, ImGuiKey_Period
			, ImGuiKey_Apostrophe // Quote
			, ImGuiKey_Slash
			, ImGuiKey_Backslash
			, ImGuiKey_None // Tilde
			, ImGuiKey_Equal
			, ImGuiKey_None // Hyphen
			, ImGuiKey_Space
			, ImGuiKey_Enter
			, ImGuiKey_Backspace
			, ImGuiKey_Tab
			, ImGuiKey_PageUp
			, ImGuiKey_PageDown
			, ImGuiKey_End
			, ImGuiKey_Home
			, ImGuiKey_Insert
			, ImGuiKey_Delete
			, ImGuiKey_None // Add
			, ImGuiKey_None // Subtract
			, ImGuiKey_None // Multiply
			, ImGuiKey_None // Divide
			, ImGuiKey_LeftArrow
			, ImGuiKey_RightArrow
			, ImGuiKey_UpArrow
			, ImGuiKey_DownArrow
			, ImGuiKey_None // Numpad0
			, ImGuiKey_None // Numpad1
			, ImGuiKey_None // Numpad2
			, ImGuiKey_None // Numpad3
			, ImGuiKey_None // Numpad4
			, ImGuiKey_None // Numpad5
			, ImGuiKey_None // Numpad6
			, ImGuiKey_None // Numpad7
			, ImGuiKey_None // Numpad8
			, ImGuiKey_None // Numpad9
			, ImGuiKey_None // NumpadDecimal
			, ImGuiKey_None // NumpadDivide
			, ImGuiKey_None // NumpadMultiply
			, ImGuiKey_None // NumpadSubstract
			, ImGuiKey_None // NumpadAdd
			, ImGuiKey_None // NumpadEnter
			, ImGuiKey_None // NumpadEqual
			, ImGuiKey_F1
			, ImGuiKey_F2
			, ImGuiKey_F3
			, ImGuiKey_F4
			, ImGuiKey_F5
			, ImGuiKey_F6
			, ImGuiKey_F7
			, ImGuiKey_F8
			, ImGuiKey_F9
			, ImGuiKey_F10
			, ImGuiKey_F11
			, ImGuiKey_F12
			, ImGuiKey_F13
			, ImGuiKey_F14
			, ImGuiKey_F15
			, ImGuiKey_F16
			, ImGuiKey_F17
			, ImGuiKey_F18
			, ImGuiKey_F19
			, ImGuiKey_F20
			, ImGuiKey_F21
			, ImGuiKey_F22
			, ImGuiKey_F23
			, ImGuiKey_F24
			, ImGuiKey_None // F25
			, ImGuiKey_Pause
			, ImGuiKey_CapsLock
			, ImGuiKey_ScrollLock
			, ImGuiKey_NumLock
			, ImGuiKey_PrintScreen
		};

		auto const keyIndex = std::to_underlying(a_key);
		if (keyIndex < 0 || keyIndex >= std::to_underlying(vob::aoein::keyboard::key::count))
		{
			return ImGuiKey_None;
		}

		return k_keyToImGuiKey[keyIndex];
	}

	void process_event(vob::aoewi::key_event const& a_keyEvent)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddKeyEvent(to_imgui_key(a_keyEvent.m_key), a_keyEvent.m_action == vob::aoewi::key_event::action::press);
	}

	void process_event(vob::aoewi::text_event const& a_textEvent)
	{
	}

	void process_event(vob::aoewi::mouse_move_event const& a_mouseMoveEvent)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddMousePosEvent(static_cast<float>(a_mouseMoveEvent.m_position.x), static_cast<float>(a_mouseMoveEvent.m_position.y));
	}

	void process_event(vob::aoewi::mouse_button_event const& a_mouseButtonEvent)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddMouseButtonEvent(std::to_underlying(a_mouseButtonEvent.m_button), a_mouseButtonEvent.m_pressed);
	}

	void process_event(vob::aoewi::mouse_scroll_event const& a_mouseScrollEvent)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddMouseWheelEvent(a_mouseScrollEvent.m_move.x, a_mouseScrollEvent.m_move.y);
	}

	void process_event(vob::aoewi::mouse_hover_event const& a_mouseHoverEvent)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddFocusEvent(a_mouseHoverEvent.m_entered);
	}

	void process_events(vob::aoewi::window_interface& a_window)
	{
		for (auto const& polledEvent : a_window.get_polled_events())
		{
			std::visit([](auto const& a_event) { process_event(a_event); }, polledEvent);
		}
	}
}

namespace vob::aoegl
{
	render_imgui_system::render_imgui_system(aoeng::world_data_provider& a_wdp)
		: m_windowWorldComponent{ a_wdp }
		, m_windowInputWorldComponent{ a_wdp }
		, m_imGuiContextWorldComponent{ a_wdp }
	{
		// Init ImGui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		aoewi::glfw_window* window = dynamic_cast<aoewi::glfw_window*>(&m_windowWorldComponent->m_window.get());
		ImGui_ImplGlfw_InitForOpenGL(window->get_native_handle(), false);
		ImGui_ImplOpenGL3_Init();

		prepare_next_frame();
	}

	render_imgui_system::~render_imgui_system()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void render_imgui_system::update() const
	{
		finalize_prev_frame();

		process_events(m_windowWorldComponent->m_window.get());

		ImGuiIO& io = ImGui::GetIO();
		m_windowInputWorldComponent->m_shouldIgnoreKeyboardKeyEvents = io.WantCaptureKeyboard;
		m_windowInputWorldComponent->m_shouldIgnoreMouseButtonEvents = io.WantCaptureMouse;

		prepare_next_frame();
	}

	
}