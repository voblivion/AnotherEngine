#pragma once

#include <vob/aoe/input/inputs.h>

#include <glm/glm.hpp>

#include <optional>
#include <span>
#include <variant>
#include <vector>


namespace vob::aoewi
{
	enum class keyboard_modifier : std::uint8_t
	{
		shift = 0
		, control
		, alt
		, super
		, caps_lock
		, num_lock
	};

	class keyboard_modifier_mask
	{
	public:
		explicit keyboard_modifier_mask(std::uint8_t a_value)
			: m_value{ a_value }
		{}

		void set(keyboard_modifier a_modifier)
		{
			m_value |= to_value(a_modifier);
		}

		void unset(keyboard_modifier a_modifier)
		{
			m_value ^= m_value & to_value(a_modifier);
		}

		bool has(keyboard_modifier a_modifier) const
		{
			return m_value & to_value(a_modifier);
		}

	private:
		std::uint8_t m_value;

		static std::uint8_t to_value(keyboard_modifier a_modifier)
		{
			return 1 << static_cast<std::uint8_t>(a_modifier);
		}
	};

	struct key_event
	{
		enum class action : std::uint8_t
		{
			release = 0
			, press
			, repeat
		};

		aoein::keyboard::key m_key;
		std::uint32_t m_scanCode;
		action m_action;
		keyboard_modifier_mask m_keyboardModifierMask;
	};

	struct text_event
	{
		char32_t m_unicode;
	};

	struct mouse_move_event
	{
		glm::ivec2 m_position;
	};

	struct mouse_hover_event
	{
		bool m_entered;
	};

	struct mouse_button_event
	{
		aoein::mouse::button m_button;
		bool m_pressed;
		keyboard_modifier_mask m_keyboardModifierMask;
	};

	struct mouse_scroll_event
	{
		glm::ivec2 m_move;
	};

	using window_event = std::variant<
		key_event
		, text_event
		, mouse_move_event
		, mouse_hover_event
		, mouse_button_event
		, mouse_scroll_event
	>;

	enum class cursor_state
	{
		normal = 0
		, hidden
		, disabled
	};

	class window_interface
	{
	public:
		virtual glm::ivec2 get_size() const = 0;
		virtual void swap_buffers() = 0;
		virtual void poll_events() = 0;
		virtual std::span<window_event const> get_polled_events() const = 0;
		virtual bool should_close() const = 0;
		virtual unsigned int get_default_framebuffer_id() const = 0;
		virtual bool is_hovered() const = 0;
		virtual void set_cursor_state(cursor_state a_cursorState) = 0;
	};
}
