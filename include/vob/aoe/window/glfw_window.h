#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/window/window_interface.h>

#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <stdexcept>

namespace vob::aoewi
{
	struct glfw_create_window_error final : std::runtime_error
	{
		glfw_create_window_error();
	};

	struct glew_initialize_error final : std::runtime_error
	{
		glew_initialize_error();
	};

	class VOB_AOE_API glfw_window final
		: public window_interface
	{
	public:
		glfw_window(
			glm::ivec2 a_size
			, char const* const a_title
			, GLFWmonitor* const a_monitor = nullptr
			, GLFWwindow* const a_share = nullptr);

		glfw_window(glfw_window&&) = delete;
		glfw_window(glfw_window const&) = delete;

		~glfw_window();

		auto& operator==(glfw_window&&) = delete;
		auto& operator==(glfw_window const&) = delete;

#pragma region window_interface
		glm::ivec2 get_size() const override;
		void swap_buffers() override;
		void poll_events() override;
		std::span<window_event const> get_polled_events() const override;
		bool should_close() const override;
		unsigned int get_default_framebuffer_id() const override;
		bool is_hovered() const override;
		void set_cursor_state(cursor_state a_cursorState) override;

		bool is_gamepad_present(int a_gamepadIndex) const override;
		bool is_gamepad_button_pressed(int a_gamepadIndex, aoein::gamepad::button a_button) const override;
		float get_gamepad_axis_value(int a_gamepadIndex, aoein::gamepad::axis a_axis) const override;
#pragma endregion

		GLFWwindow* get_native_handle() const;

	private:
		GLFWwindow* m_nativeHandle = nullptr;
		std::pmr::vector<window_event> m_events;

		static void key_event_callback(GLFWwindow*, GLint, GLint, GLint, GLint);
		static void text_event_callback(GLFWwindow*, GLuint);
		static void mouse_move_event_callback(GLFWwindow*, GLdouble, GLdouble);
		static void mouse_enter_event_callback(GLFWwindow*, GLint);
		static void mouse_button_event_callback(GLFWwindow*, GLint, GLint, GLint);
		static void mouse_scroll_event_callback(GLFWwindow*, GLdouble, GLdouble);
		static void framebuffer_size_callback(GLFWwindow*, GLint, GLint);
#ifndef NDEBUG
		static void debug_message_callback(
			GLenum, GLenum, GLuint, GLenum, GLsizei, GLchar const*, void const*);
#endif

		void push_event(window_event a_event);
	};
}
