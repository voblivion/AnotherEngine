#pragma once

#include <array>
#include <SFML/Window/Keyboard.hpp>

#include <aoe/common/window/Button.h>

namespace aoe
{
	namespace common
	{
		struct Keyboard
		{
			std::array<Button, sf::Keyboard::KeyCount> m_keys;
		};
	}
}