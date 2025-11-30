#include <vob/aoe/rendering/PrepareImGuiFrameSystem.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


namespace vob::aoegl
{
	namespace
	{
		ImGuiKey toImGuiKey(vob::aoein::Keyboard::Key a_key)
		{
			constexpr std::array<ImGuiKey, std::to_underlying(vob::aoein::Keyboard::Key::count)> k_keyToImGuiKey{
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
			if (keyIndex < 0 || keyIndex >= std::to_underlying(vob::aoein::Keyboard::Key::count))
			{
				return ImGuiKey_None;
			}

			return k_keyToImGuiKey[keyIndex];
		}

		void processEvent(vob::aoewi::KeyEvent const& a_keyEvent)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.AddKeyEvent(toImGuiKey(a_keyEvent.key), a_keyEvent.action == vob::aoewi::KeyEvent::Action::Press);
		}

		void processEvent(vob::aoewi::TextEvent const& a_textEvent)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.AddInputCharacter(a_textEvent.unicode);
		}

		void processEvent(vob::aoewi::MouseMoveEvent const& a_mouseMoveEvent)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.AddMousePosEvent(static_cast<float>(a_mouseMoveEvent.position.x), static_cast<float>(a_mouseMoveEvent.position.y));
		}

		void processEvent(vob::aoewi::MouseButtonEvent const& a_mouseButtonEvent)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.AddMouseButtonEvent(std::to_underlying(a_mouseButtonEvent.button), a_mouseButtonEvent.pressed);
		}

		void processEvent(vob::aoewi::MouseScrollEvent const& a_mouseScrollEvent)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.AddMouseWheelEvent(static_cast<float>(a_mouseScrollEvent.move.x), static_cast<float>(a_mouseScrollEvent.move.y));
		}

		void processEvent(vob::aoewi::MouseHoverEvent const& a_mouseHoverEvent)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.AddFocusEvent(a_mouseHoverEvent.entered);
		}
		void processEvents(vob::aoewi::IWindow& a_window)
		{
			for (auto const& polledEvent : a_window.getPolledEvents())
			{
				std::visit([](auto const& a_event) { processEvent(a_event); }, polledEvent);
			}
		}
	}

	void PrepareImGuiFrameSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_windowInputContext.init(a_wdar);
		m_windowContext.init(a_wdar);
		m_imGuiContext.init(a_wdar);
	}

	void PrepareImGuiFrameSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		processEvents(m_windowContext.get(a_wdap).window.get());

		ImGuiIO& io = ImGui::GetIO();
		auto& windowInputContext = m_windowInputContext.get(a_wdap);
		windowInputContext.shouldIgnoreKeyboardKeyEvents = io.WantCaptureKeyboard;
		windowInputContext.shouldIgnoreMouseButtonEvents = io.WantCaptureMouse;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}


}