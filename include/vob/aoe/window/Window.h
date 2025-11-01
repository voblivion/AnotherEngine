#pragma once

#include <vob/aoe/input/Inputs.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <span>
#include <variant>


namespace vob::aoewi
{
	enum class KeyboardModifier : uint8_t
	{
		Shift,
		Control,
		Alt,
		Super,
		CapsLock,
		NumLock
	};

	class KeyboardModifierMask
	{
	public:
		explicit KeyboardModifierMask(uint8_t a_value)
			: m_value{ a_value }
		{
		}

		void set(KeyboardModifier a_modifier)
		{
			m_value |= toValue(a_modifier);
		}

		void unset(KeyboardModifier a_modifier)
		{
			m_value ^= m_value & toValue(a_modifier);
		}

		bool has(KeyboardModifier a_modifier) const
		{
			return (m_value & toValue(a_modifier)) != 0;
		}

	private:
		uint8_t m_value;

		static uint8_t toValue(KeyboardModifier a_modifier)
		{
			return 1 << static_cast<uint8_t>(a_modifier);
		}
	};

	struct KeyEvent
	{
		enum class Action : uint8_t
		{
			Release = 0,
			Press,
			Repeat
		};

		aoein::Keyboard::Key key;
		std::uint32_t scanCode;
		Action action;
		KeyboardModifierMask keyboardModifierMask;
	};

	struct TextEvent
	{
		char32_t unicode;
	};

	struct MouseMoveEvent
	{
		glm::ivec2 position;
	};

	struct MouseHoverEvent
	{
		bool entered;
	};

	struct MouseButtonEvent
	{
		aoein::Mouse::Button button;
		bool pressed;
		KeyboardModifierMask keyboardModifierMask;
	};

	struct MouseScrollEvent
	{
		glm::ivec2 move;
	};

	using WindowEvent = std::variant<
		KeyEvent
		, TextEvent
		, MouseMoveEvent
		, MouseHoverEvent
		, MouseButtonEvent
		, MouseScrollEvent
	>;

	enum class CursorState
	{
		Normal = 0
		, Hidden
		, Disabled
	};

	struct IWindow
	{
		virtual glm::ivec2 getSize() const = 0;
		virtual void swapBuffers() = 0;
		virtual void pollEvents() = 0;
		virtual std::span<WindowEvent const> getPolledEvents() const = 0;
		virtual bool shouldClose() const = 0;
		virtual uint32_t getDefaultFramebufferId() const = 0;
		virtual bool isHovered() const = 0;
		virtual void setCursorState(CursorState a_cursorState) = 0;

		virtual bool isGamepadPresent(int32_t a_gamepadIndex) const = 0;
		virtual bool isGamepadButtonPressed(int32_t a_gamepadIndex, aoein::Gamepad::Button a_button) const = 0;
		virtual float getGamepadAxisValue(int32_t a_gamepadIndex, aoein::Gamepad::Axis a_axis) const = 0;
	};
}
