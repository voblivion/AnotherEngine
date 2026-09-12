#pragma once

#include "vob/aoe/window/GlfwMonitors.h"
#include "vob/aoe/window/Window.h"

#include "GLFW/glfw3.h"
#include "gl/glew.h"
#include "glm/glm.hpp"

#include <stdexcept>
#include <vector>

namespace vob::aoewi
{
	struct GlfwCreateWindowError final : std::runtime_error
	{
		GlfwCreateWindowError();
	};

	struct GlewInitializeError final : std::runtime_error
	{
		GlewInitializeError();
	};

	class GlfwWindow final : public IWindow
	{
	  public:
		explicit GlfwWindow(
			glm::ivec2 a_size,
			char const* const a_title,
			bool const a_isDecorated = false,
			GLFWmonitor* const a_monitor = nullptr,
			GLFWwindow* const a_share = nullptr);

		GlfwWindow(GlfwWindow&&) = delete;
		GlfwWindow(GlfwWindow const&) = delete;

		~GlfwWindow();

		auto& operator=(GlfwWindow&&) = delete;
		auto& operator=(GlfwWindow const&) = delete;

#pragma region IWindow
		IMonitors const& getMonitors() const override;
		glm::ivec2 getSize() const override;
		glm::ivec2 getPosition() const override;
		DisplayMode getActiveDisplayMode() const override;
		void requestDisplayModeChange(DisplayMode const& a_displayMode) override;

		void pollEvents() override;
		std::span<WindowEvent const> getPolledEvents() const override;
		uint32_t getDefaultFramebufferId() const override;
		void setVSyncEnabled(bool a_enabled) override;
		void swapBuffers() override;
		bool shouldClose() const override;

		bool isHovered() const override;
		void setCursorState(CursorState a_cursorState) override;
		glm::vec2 getMousePosition() const override;
		bool isGamepadPresent(int32_t a_gamepadIndex) const override;
		bool isGamepadButtonPressed(int32_t a_gamepadIndex, aoein::Gamepad::Button a_button) const override;
		float getGamepadAxisValue(int32_t a_gamepadIndex, aoein::Gamepad::Axis a_axis) const override;
#pragma endregion

		GLFWwindow* getNativeHandle() const;

	  private:
		struct KnownMonitor
		{
			GLFWmonitor* handle = nullptr;
			std::string id;
		};

		GLFWwindow* m_nativeHandle = nullptr;
		GlfwMonitors m_monitors;
		std::pmr::vector<WindowEvent> m_events;
		std::vector<KnownMonitor> m_knownMonitors;
		DisplayMode m_desiredDisplayMode;
		DisplayMode m_lastDisplayMode;
		bool m_hasRequestedDisplayModeChange = false;

		static void keyEventCallback(GLFWwindow*, GLint, GLint, GLint, GLint);
		static void textEventCallback(GLFWwindow*, GLuint);
		static void mouseMoveEventCallback(GLFWwindow*, GLdouble, GLdouble);
		static void mouseEnterEventCallback(GLFWwindow*, GLint);
		static void mouseButtonEventCallback(GLFWwindow*, GLint, GLint, GLint);
		static void mouseScrollEventCallback(GLFWwindow*, GLdouble, GLdouble);
		void applyRequestedDisplayMode();
		void reconcileMonitorChanges();
		GLFWmonitor* getMonitorHandle(std::string_view a_monitorId) const;
#ifndef NDEBUG
		static void debugMessageCallback(GLenum, GLenum, GLuint, GLenum, GLsizei, GLchar const*, void const*);
#endif

		void pushEvent(WindowEvent a_event);
	};
}
