#pragma once

#include <optional>

#include <SFML/Graphics/Image.hpp>

#include <vob/aoe/api.h>
#include <vob/aoe/common/render/OpenGl.h>

namespace vob::aoe::common
{
	class VOB_AOE_API Texture
	{
	public:
		// Types
		struct State
		{
			GraphicObjectId m_textureId;
		};

		// Attributes
		mutable std::optional<State> m_state;
		sf::Image m_source;


		// Constructors
		explicit Texture(sf::Image a_source);

		// Methods
		auto getTextureId() const
		{
			return m_state.value().m_textureId;
		}
		void bind(GLenum a_target) const
		{
			glBindTexture(a_target, getTextureId());
		}
		bool isReady() const
		{
			return m_state.has_value();
		}
		void setSource(sf::Image a_source)
		{
			m_source = std::move(a_source);
		}

		void create() const;
		void destroy() const;
	};
}
