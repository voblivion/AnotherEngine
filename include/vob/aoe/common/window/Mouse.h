#pragma once

#include <array>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Mouse.hpp>

#include <vob/aoe/common/window/Button.h>

namespace vob::aoe::common
{
	struct Mouse
	{
		sf::Vector2i m_position{};
		sf::Vector2i m_move{};
		std::array<Button, sf::Mouse::ButtonCount> m_buttons;
	};
}
