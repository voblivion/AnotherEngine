#pragma once
#include "vob/aoe/core/ecs/Component.h"
#include <SFML/Window/Window.hpp>

namespace vob::aoe::common
{
	class WindowComponent final
		: public ecs::AComponent
	{
	public:
		// Attributes
		std::reference_wrapper<sf::Window> m_window;

		// Constructors
		explicit WindowComponent(sf::Window& a_window)
			: m_window{ a_window }
		{}

		// Methods
		sf::Window& getWindow() const
		{
			return m_window;
		}
	};
}
