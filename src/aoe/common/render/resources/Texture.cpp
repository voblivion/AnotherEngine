#include <vob/aoe/common/_render/resources/Texture.h>


namespace vob::aoe::common
{
	Texture::Texture(sf::Image a_source)
		: m_source{ a_source }
	{}

	void Texture::create() const
	{
		m_state = State{};
		auto& state = m_state.value();

		glCreateTextures(GL_TEXTURE_2D, 1, &state.m_textureId);
		glBindTexture(GL_TEXTURE_2D, state.m_textureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(
			GL_TEXTURE_2D
			, 0
			, GL_RGBA
			, m_source.getSize().x
			, m_source.getSize().y
			, 0
			, GL_RGBA
			, GL_UNSIGNED_BYTE
			, m_source.getPixelsPtr()
		);
	}

	void Texture::destroy() const
	{
		auto& state = m_state.value();

		glDeleteTextures(1, &state.m_textureId);

		m_state.reset();
	}

}
