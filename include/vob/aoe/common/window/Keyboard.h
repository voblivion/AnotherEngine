#pragma once

#include <array>
#include <SFML/Window/Keyboard.hpp>

#include <vob/aoe/common/window/Button.h>

namespace vob::aoe::common
{
	struct Keyboard
	{
		std::array<Button, sf::Keyboard::KeyCount> m_keys;
	};
}