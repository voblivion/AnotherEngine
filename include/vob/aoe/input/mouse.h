#pragma once

#include <vob/aoe/input/switch_input.h>

#include <vob/misc/std/enum_map.h>

#include <glm/glm.hpp>


namespace vob::aoein
{
	class mouse
	{
	public:
		enum class button
		{
			unknown = -1,
			M1 = 0,
			M2,
			M3,
			M4,
			M5,
			count,

			// Named
			Left = M1,
			Right = M2,
			Middle = M3,
			X1 = M4,
			X2 = M5
		};

		glm::vec2 m_position = {};
		glm::vec2 m_move = {};
		switch_input m_hover = {};
		mistd::enum_map<button, button::M1, button::count, switch_input> m_buttons;

		void update()
		{
			for (auto& button : m_buttons)
			{
				button.update();
			}
		}
	};
}
