#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <iostream>

#include <aoe/common/render/GlObjects.h>

namespace aoe::common
{
	struct RenderTexture
	{
		void init(std::uint32_t const a_width, std::uint32_t const a_height)
		{
			m_framebuffer.tryCreate();
			gl::ResourceScopeUse<gl::Framebuffer> t_framebufferUse{ m_framebuffer };

			m_texture.tryCreate();
			gl::ResourceScopeUse<gl::Texture> t_textureUse{ m_texture };
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, a_width, a_height, 0, GL_RGB
				, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0
				, GL_TEXTURE_2D, m_texture.m_id, 0);

			m_renderbuffer.tryCreate();
			gl::ResourceScopeUse<gl::Renderbuffer> t_renderbufferUse{ m_renderbuffer };
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, a_width
				, a_height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT
				, GL_RENDERBUFFER, m_renderbuffer.m_id);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				std::cerr << "Invalid Frame Buffer" << std::endl;
				m_renderbuffer.tryRelease();
				m_texture.tryRelease();
				m_framebuffer.tryRelease();
			}
		}

		void startRenderTo() const
		{
			m_framebuffer.startUsing();
		}

		void stopRenderTo() const
		{
			m_framebuffer.stopUsing();
		}

		gl::Framebuffer m_framebuffer;
		gl::Texture m_texture;
		gl::Renderbuffer m_renderbuffer;
	};
}
