#pragma once

#include <array>

#include <vob/sta/enum_map.h>

#include <vob/aoe/common/space/Vector.h>
#include <vob/aoe/common/input/physical/Switch.h>

namespace vob::aoe::common
{
	class Mouse
	{
	public:
		enum class Button
		{
			Unknown = -1
			, M1 = 0
			, M2
			, M3
			, M4
			, M5
			, Count

			, Left = M1
			, Right = M2
			, Middle = M3
			, X1 = M4
			, X2 = M5
		};

		vec2 m_position = {};
		vec2 m_move = {};
		Switch m_hover = {};
		sta::enum_map<Button, Button::M1, Button::Count, Switch> m_buttons{};
	};

	inline Mouse::Button mouseButtonFromGlfw(int a_glfwMouseButtonId)
	{
		if (a_glfwMouseButtonId < GLFW_MOUSE_BUTTON_1 || a_glfwMouseButtonId > GLFW_MOUSE_BUTTON_8)
		{
			return Mouse::Button::Unknown;
		}

		return static_cast<Mouse::Button>(a_glfwMouseButtonId);
	}
}
