#pragma once

#include <vob/aoe/input/gamepad.h>
#include <vob/aoe/input/keyboard.h>
#include <vob/aoe/input/mouse.h>

#include <array>


namespace vob::aoein
{
	struct physical_inputs
	{
		// Types
		using gamepad_idx = std::uint8_t;

		enum class device
		{
			keyboard,
			gamepad,
			mouse
		};

		// Class Data
		static constexpr std::size_t k_max_gamepad_count = 16;

		// Data
		std::array<gamepad, k_max_gamepad_count> m_gamepads;
		keyboard m_keyboard;
		mouse m_mouse;

		void update()
		{
			for (auto& gamepad : m_gamepads)
			{
				gamepad.update();
			}
			m_keyboard.update();
			m_mouse.update();
		}
	};
}
