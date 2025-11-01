#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/window/Window.h>

#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

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

	class VOB_AOE_API GlfwWindow final : public IWindow
	{
	public:
		explicit GlfwWindow(
			glm::ivec2 a_size
			, char const* const a_title
			, GLFWmonitor* const a_monitor = nullptr
			, GLFWwindow* const a_share = nullptr);

		GlfwWindow(GlfwWindow&&) = delete;
		GlfwWindow(GlfwWindow const&) = delete;

		~GlfwWindow();

		auto& operator==(GlfwWindow&&) = delete;
		auto& operator==(GlfwWindow const&) = delete;

#pragma region IWindow
		glm::ivec2 getSize() const override;
		void swapBuffers() override;
		void pollEvents() override;
		std::span<WindowEvent const> getPolledEvents() const override;
		bool shouldClose() const override;
		uint32_t getDefaultFramebufferId() const override;
		bool isHovered() const override;
		void setCursorState(CursorState a_cursorState) override;

		bool isGamepadPresent(int32_t a_gamepadIndex) const override;
		bool isGamepadButtonPressed(int32_t a_gamepadIndex, aoein::Gamepad::Button a_button) const override;
		float getGamepadAxisValue(int32_t a_gamepadIndex, aoein::Gamepad::Axis a_axis) const override;
#pragma endregion

		GLFWwindow* getNativeHandle() const;

	private:
		GLFWwindow* m_nativeHandle = nullptr;
		std::pmr::vector<WindowEvent> m_events;

		static void keyEventCallback(GLFWwindow*, GLint, GLint, GLint, GLint);
		static void textEventCallback(GLFWwindow*, GLuint);
		static void mouseMoveEventCallback(GLFWwindow*, GLdouble, GLdouble);
		static void mouseEnterEventCallback(GLFWwindow*, GLint);
		static void mouseButtonEventCallback(GLFWwindow*, GLint, GLint, GLint);
		static void mouseScrollEventCallback(GLFWwindow*, GLdouble, GLdouble);
		static void frameBufferSizeCallback(GLFWwindow*, GLint, GLint);
#ifndef NDEBUG
		static void debugMessageCallback(
			GLenum, GLenum, GLuint, GLenum, GLsizei, GLchar const*, void const*);
#endif

		void pushEvent(WindowEvent a_event);
	};
}
