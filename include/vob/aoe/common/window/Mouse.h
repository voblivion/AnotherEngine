#pragma once

#include <array>

#include <SFML/Window/Mouse.hpp>

#include <vob/sta/enum_map.h>

#include <vob/aoe/common/space/Vector.h>
#include <vob/aoe/common/window/Switch.h>

namespace vob::aoe::common
{
	struct Mouse
	{
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

		vec2 m_position{};
		vec2 m_move{};
		Switch m_hover;
		sta::enum_map<Button, Switch, Button::M1, Button::Count> m_buttons;
	};

	inline Mouse::Button toButton(int a_glfwButtonId)
	{
		constexpr std::array<Mouse::Button, GLFW_MOUSE_BUTTON_8 + 1 - GLFW_MOUSE_BUTTON_1> s_source{
			Mouse::Button::Left // 0 GLFW_MOUSE_BUTTON_1 | GLFW_MOUSE_BUTTON_LEFT
			, Mouse::Button::Right // 1 GLFW_MOUSE_BUTTON_2 | GLFW_MOUSE_BUTTON_RIGHT
			, Mouse::Button::Middle // 2 GLFW_MOUSE_BUTTON_3 | GLFW_MOUSE_BUTTON_MIDDLE
			, Mouse::Button::X1 // 3 GLFW_MOUSE_BUTTON_4
			, Mouse::Button::X2 // 4 GLFW_MOUSE_BUTTON_5
			, Mouse::Button::Unknown // 5 GLFW_MOUSE_BUTTON_6
			, Mouse::Button::Unknown // 6 GLFW_MOUSE_BUTTON_7
			, Mouse::Button::Unknown // 7 GLFW_MOUSE_BUTTON_8
		};

		if (a_glfwButtonId < GLFW_MOUSE_BUTTON_1 || a_glfwButtonId > GLFW_MOUSE_BUTTON_8)
		{
			return Mouse::Button::Unknown;
		}

		return s_source[a_glfwButtonId - GLFW_MOUSE_BUTTON_1];
	}
}
