#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <iostream>

#include <vob/aoe/common/opengl/resources/Object.h>

namespace vob::aoe::ogl
{
	struct RenderTexture
	{
		RenderTexture(std::size_t const a_width, std::size_t const a_height)
			: m_width{ a_width }
			, m_height{ a_height }
		{
		}

		void create() const
		{
			m_framebuffer.create();
			glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer.m_id);

			m_texture.create();
			glBindTexture(GL_TEXTURE_2D, m_texture.m_id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<std::uint32_t>(m_width), static_cast<std::int32_t>(m_height), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture.m_id, 0);

			m_renderbuffer.create();
			glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer.m_id);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<std::int32_t>(m_width), static_cast<std::int32_t>(m_height));
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderbuffer.m_id);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				std::cerr << "Invalid Frame Buffer" << std::endl;
				destroy();
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			auto error = glGetError();
			if (error != GL_NO_ERROR)
			{
				error += 1;
			}
		}

		bool isReady() const
		{
			return m_renderbuffer.isReady();
		}

		void destroy() const
		{
			m_renderbuffer.destroy();
			m_texture.destroy();
			m_framebuffer.destroy();
		}

		void startRenderTo() const
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer.m_id);
		}

		void stopRenderTo() const
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		std::size_t m_width;
		std::size_t m_height;

		FramebufferObject m_framebuffer;
		TextureObject m_texture;
		RenderbufferObject m_renderbuffer;
	};
}
