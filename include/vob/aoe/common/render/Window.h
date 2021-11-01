#pragma once

#include <deque>
#include <variant>

#include <vob/aoe/common/render/IWindow.h>

#define GLFW_DLL
#include <GLFW/glfw3.h>


namespace vob::aoe::common
{
	struct GlfwCreateWindowError final
		: std::runtime_error
	{
		GlfwCreateWindowError()
			: std::runtime_error{ "Failed to create Glfw window" }
		{}
	};

	struct GlewInitializationError final
		: std::runtime_error
	{
		GlewInitializationError()
			: std::runtime_error{ "Failed to initialize Glew context" }
		{}
	};

	class Window final
		: public IWindow
	{
	public:
		// Constructors / Destructor
		explicit Window(
			std::size_t a_width
			, std::size_t a_height
			, char const* const a_title
			, GLFWmonitor* const a_monitor = nullptr
			, GLFWwindow* const a_share = nullptr
		)
		{
			m_nativeHandle = glfwCreateWindow(
				static_cast<int>(a_width)
				, static_cast<int>(a_height)
				, a_title
				, a_monitor
				, a_share
			);

			if (m_nativeHandle == nullptr)
			{
				throw GlfwCreateWindowError{};
			}

			glfwMakeContextCurrent(m_nativeHandle);

			// Initialize current context
			if (glewInit() != GLEW_OK)
			{
				throw GlewInitializationError{};
			}

			// Set event callbacks
			glfwSetWindowUserPointer(m_nativeHandle, this);
			glfwSetKeyCallback(m_nativeHandle, keyEventCallback);
			glfwSetCharCallback(m_nativeHandle, textEventCallback);
			glfwSetCursorPosCallback(m_nativeHandle, mouseMoveEventCallback);
			glfwSetCursorEnterCallback(m_nativeHandle, mouseEnterEventCallback);
			glfwSetMouseButtonCallback(m_nativeHandle, mouseButtonEventCallback);
			glfwSetScrollCallback(m_nativeHandle, mouseScrollEventCallback);
			
			glfwSetFramebufferSizeCallback(m_nativeHandle, framebufferSizeCallback);

#ifndef NDEBUG
			// Set error callback
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(debugMessageCallback, this);
			GLuint ids = 0;
			glDebugMessageControl(
				GL_DONT_CARE
				, GL_DONT_CARE
				, GL_DONT_CARE
				, 0
				, &ids
				, true
			);
#endif
		}
		Window(Window&&) = delete;
		Window(Window const&) = delete;
		~Window()
		{
			glfwDestroyWindow(m_nativeHandle);
		}

		// Operators
		auto& operator=(Window&&) = delete;
		auto& operator=(Window const&) = delete;

		// Methods
		glm::ivec2 getSize() const override
		{
			auto size = glm::ivec2{};
			glfwGetWindowSize(m_nativeHandle, &size.x, &size.y);
			return size;
		}

		void swapBuffer() override
		{
			glfwSwapBuffers(m_nativeHandle);
		}

		void pollEvents() override
		{
			m_events.clear();
			glfwPollEvents();
		}

		const std::vector<WindowEvent>& getPolledEvents() const override
		{
			return m_events;
		}

		bool shouldClose() const override
		{
			return glfwWindowShouldClose(m_nativeHandle) == GLFW_TRUE;
		}

		GraphicObjectId getDefaultFramebufferId() const override
		{
			return 0;
		}

		bool isHovered() const override
		{
			return glfwGetWindowAttrib(m_nativeHandle, GLFW_HOVERED) == GLFW_TRUE;
		}

		void setCursorState(CursorState a_cursorState) override
		{
			glfwSetInputMode(
				m_nativeHandle
				, GLFW_CURSOR
				, static_cast<GLenum>(a_cursorState)
			);
		}

		GLFWwindow* getNativeHandle() const
		{
			return m_nativeHandle;
		}

		void activate()
		{
			glfwMakeContextCurrent(m_nativeHandle);
		}

	private:
		// Attributes
		GLFWwindow* m_nativeHandle = nullptr;
		bool m_needPollEvents = true;
		std::vector<WindowEvent> m_events;

		// Methods
		void addEvent(WindowEvent a_windowEvent)
		{
			m_events.emplace_back(std::move(a_windowEvent));
		}

		// Static methods
		static void framebufferSizeCallback(
			GLFWwindow* a_windowNativeHandle
			, GLint a_width
			, GLint a_height
		)
		{
		}

		static void debugMessageCallback(
			GLenum a_source
			, GLenum a_type
			, GLuint a_id
			, GLenum a_severity
			, GLsizei a_length
			, GLchar const* a_message
			, void const* a_userParam
		)
		{
			// ignore non-significant error/warning codes
			if (a_id == 131169 || a_id == 131185 || a_id == 131218 || a_id == 131204) return;

			std::cerr << "[";
			switch (a_type)
			{
			case GL_DEBUG_TYPE_ERROR:
				std::cerr << "Error";
				break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
				std::cerr << "Deprecated Behaviour";
				break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
				std::cerr << "Undefined Behaviour";
				break;
			case GL_DEBUG_TYPE_PORTABILITY:
				std::cerr << "Portability";
				break;
			case GL_DEBUG_TYPE_PERFORMANCE:
				std::cerr << "Performance";
				break;
			case GL_DEBUG_TYPE_MARKER:
				std::cerr << "Marker";
				break;
			case GL_DEBUG_TYPE_PUSH_GROUP:
				std::cerr << "Push Group";
				break;
			case GL_DEBUG_TYPE_POP_GROUP:
				std::cerr << "Pop Group";
				break;
			case GL_DEBUG_TYPE_OTHER:
				std::cerr << "Other";
				break;
			default:
				std::cerr << "?";
				break;
			}
			std::cerr << "][";
			switch (a_severity)
			{
			case GL_DEBUG_SEVERITY_HIGH:
				std::cerr << "HIGH";
				break;
			case GL_DEBUG_SEVERITY_MEDIUM:
				std::cerr << "MEDIUM";
				break;
			case GL_DEBUG_SEVERITY_LOW:
				std::cerr << "LOW";
				break;
			case GL_DEBUG_SEVERITY_NOTIFICATION:
				std::cerr << "NOTIFICATION";
				break;
			default:
				std::cerr << "?";
				break;
			}
			std::cerr << "] ";
			switch (a_source)
			{
			case GL_DEBUG_SOURCE_API:
				std::cerr << "API";
				break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
				std::cerr << "Window System";
				break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER:
				std::cerr << "Shader Compiler";
				break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:
				std::cerr << "Third Party";
				break;
			case GL_DEBUG_SOURCE_APPLICATION:
				std::cerr << "Application";
				break;
			case GL_DEBUG_SOURCE_OTHER:
				std::cerr << "Other";
				break;
			default:
				std::cerr << "?";
				break;
			}
			std::cerr << " (" << a_id << "): " << a_message << std::endl;

			ignorable_assert(a_type != GL_DEBUG_TYPE_ERROR);
		}

		static void keyEventCallback(
			GLFWwindow* a_windowNativeHandle
			, GLint a_keyCode
			, GLint a_scanCode
			, GLint a_action
			, GLint a_modifierMask
		)
		{
			auto const window = static_cast<Window*>(glfwGetWindowUserPointer(a_windowNativeHandle));
			window->addEvent(KeyEvent{
				static_cast<std::uint32_t>(a_keyCode)
				, static_cast<std::uint32_t>(a_scanCode)
				, static_cast<KeyEvent::Action>(a_action)
				, ModifierMask{ static_cast<std::uint8_t>(a_modifierMask) }
			});
		}

		static void textEventCallback(
			GLFWwindow* a_windowNativeHandle
			, GLuint a_unicode
		)
		{
			auto const window = static_cast<Window*>(glfwGetWindowUserPointer(a_windowNativeHandle));
			window->addEvent(TextEvent{ a_unicode });
		}

		static void mouseMoveEventCallback(
			GLFWwindow* a_windowNativeHandle
			, GLdouble a_x
			, GLdouble a_y
		)
		{
			auto const window = static_cast<Window*>(glfwGetWindowUserPointer(a_windowNativeHandle));
			window->addEvent(MouseMoveEvent{{ a_x, a_y }});
		}

		static void mouseEnterEventCallback(
			GLFWwindow* a_windowNativeHandle
			, GLint a_entered
		)
		{
			auto const window = static_cast<Window*>(glfwGetWindowUserPointer(a_windowNativeHandle));
			window->addEvent(MouseEnterEvent{ a_entered == GL_TRUE });
		}

		static void mouseButtonEventCallback(
			GLFWwindow* a_windowNativeHandle
			, GLint a_button
			, GLint a_action
			, GLint a_modifierMask
		)
		{
			auto const window = static_cast<Window*>(glfwGetWindowUserPointer(a_windowNativeHandle));
			window->addEvent(MouseButtonEvent{
				static_cast<std::uint32_t>(a_button)
				, a_action == GLFW_PRESS
				, ModifierMask{ static_cast<std::uint8_t>(a_modifierMask) }
			});
			
		}

		static void mouseScrollEventCallback(
			GLFWwindow* a_windowNativeHandle
			, GLdouble a_x
			, GLdouble a_y
		)
		{
			auto const window = static_cast<Window*>(glfwGetWindowUserPointer(a_windowNativeHandle));
			window->addEvent(MouseScrollEvent{{ a_x, a_y }});
		}
	};
}
