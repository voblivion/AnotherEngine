#include <vob/aoe/common/_render/resources/RenderTexture.h>


namespace vob::aoe::common
{
	RenderTexture::RenderTexture(glm::ivec2 const a_size, std::size_t a_multiSampling)
		: m_size{ a_size }
		, m_multiSampling{ a_multiSampling }
	{
		
	}

	void RenderTexture::create() const
	{
		assert(!isReady());
		m_state.m_isReady = true;

		// Texture
		glGenTextures(1, &m_state.m_textureId);

		auto const width = static_cast<GLsizei>(m_size.x * std::sqrt(m_multiSampling));
		auto const height = static_cast<GLsizei>(m_size.y * std::sqrt(m_multiSampling));

		glBindTexture(GL_TEXTURE_2D, m_state.m_textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		//glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_state.m_textureId);
		//glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height), GL_TRUE);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Renderbuffer
		glGenRenderbuffers(1, &m_state.m_renderbufferId);
		glBindRenderbuffer(GL_RENDERBUFFER, m_state.m_renderbufferId);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		//glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height));

		// Framebuffer
		glGenFramebuffers(1, &m_state.m_framebufferId);
		glBindFramebuffer(GL_FRAMEBUFFER, m_state.m_framebufferId);
		
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_state.m_textureId, 0);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_state.m_textureId, 0);
		
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_state.m_renderbufferId);

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}

	void RenderTexture::destroy() const
	{
		assert(isReady());

		glDeleteFramebuffers(1, &m_state.m_framebufferId);
		glDeleteRenderbuffers(1, &m_state.m_renderbufferId);
		glDeleteTextures(1, &m_state.m_textureId);

		m_state.m_isReady = false;
	}
}

