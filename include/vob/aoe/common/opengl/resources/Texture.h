#pragma once

#include <optional>
#include <SFML/Graphics/Texture.hpp>
#include <vob/aoe/core/type/ADynamicType.h>

namespace vob::aoe::ogl
{
	class Texture
	{
	public:
		// Attributes
		// TODO : custom
		sf::Image m_source;
		mutable std::optional<sf::Texture> m_texture;

		// Constructors
		explicit Texture(sf::Image a_source)
			: m_source{ std::move(a_source) }
		{}

		void setSource(sf::Image a_source)
		{
			m_source = std::move(a_source);
		}

		void create() const
		{
			m_texture = sf::Texture{};
			m_texture.value().loadFromImage(m_source);
			// m_source = std::nullopt;
		}

		bool isReady() const
		{
			return m_texture != std::nullopt;
		}

		void destroy() const
		{
			if (m_texture.has_value())
			{
				m_texture = std::nullopt;
			}
		}
	};
}
