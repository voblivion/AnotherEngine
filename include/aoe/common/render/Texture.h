#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <aoe/core/standard/ADynamicType.h>

namespace aoe
{
	namespace common
	{
		class Texture final
			: public sta::ADynamicType
		{
		public:
			// Attributes
			sf::Texture m_texture;

			// Constructors
			explicit Texture(sf::Texture a_texture)
				: m_texture{ std::move(a_texture) }
			{}
		};
	}
}
