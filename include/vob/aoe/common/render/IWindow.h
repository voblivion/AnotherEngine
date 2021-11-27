#pragma once

#include <optional>
#include <variant>
#include <vector>

#include <vob/aoe/common/render/OpenGl.h>

namespace vob::aoe::common
{
	enum class Modifier : std::uint8_t
	{
		Shift = 0
		, Control
		, Alt
		, Super
		, CapsLock
		, NumLock
	};

	class ModifierMask
	{
	public:
		explicit ModifierMask(std::uint8_t a_value)
			: m_value{ a_value }
		{}

		void setModifier(Modifier a_modifier)
		{
			m_value |= toValue(a_modifier);
		}

		void unsetModifier(Modifier a_modifier)
		{
			m_value ^= m_value | toValue(a_modifier);
		}

		bool hasModifier(Modifier a_modifier) const
		{
			return m_value & toValue(a_modifier);
		}

	private:
		std::uint8_t m_value;

		static std::uint8_t toValue(Modifier a_modifier)
		{
			return 1 << static_cast<std::uint8_t>(a_modifier);
		}
	};

	struct KeyEvent
	{
		enum class Action : std::uint8_t
		{
			Release = GLFW_RELEASE
			, Press = GLFW_PRESS
			, Repeat = GLFW_REPEAT
		};

		// Attributes
		std::uint32_t m_keyCode;
		std::uint32_t m_scanCode;
		Action m_action;
		ModifierMask m_modifierMask;
	};

	struct TextEvent
	{
		char32_t m_unicode;
	};

	struct MouseMoveEvent
	{
		glm::ivec2 m_position;
	};

	struct MouseEnterEvent
	{
		bool m_entered;
	};

	struct MouseButtonEvent
	{
		std::uint32_t m_button;
		bool m_pressed;
		ModifierMask m_modifierMask;
	};

	struct MouseScrollEvent
	{
		glm::ivec2 m_move;
	};

	using WindowEvent = std::variant<
		KeyEvent
		, TextEvent
		, MouseMoveEvent
		, MouseEnterEvent
		, MouseButtonEvent
		, MouseScrollEvent
	>;

	enum class CursorState
	{
		Normal = GLFW_CURSOR_NORMAL
		, Hidden = GLFW_CURSOR_HIDDEN
		, Disable = GLFW_CURSOR_DISABLED
	};

	class IWindow
	{
	public:
		// Constructors / Destructor
		IWindow() = default;
		IWindow(IWindow&&) = default;
		IWindow(IWindow const&) = default;
		virtual ~IWindow() = default;

		// Methods
		virtual glm::ivec2 getSize() const = 0;
		virtual void swapBuffer() = 0;
		virtual void pollEvents() = 0;
		virtual const std::vector<WindowEvent>& getPolledEvents() const = 0;
		virtual bool shouldClose() const = 0;
		virtual GraphicObjectId getDefaultFramebufferId() const = 0;
		virtual bool isHovered() const = 0;
		virtual void setCursorState(CursorState a_cursorState) = 0;
	};
}
