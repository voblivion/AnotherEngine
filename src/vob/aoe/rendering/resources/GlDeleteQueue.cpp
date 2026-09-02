#include "vob/aoe/rendering/resources/GlDeleteQueue.h"

#include "vob/misc/std/container_util.h"


namespace vob::aoegl
{
	void GlDeleteQueue::pushBuffer(GraphicId const a_id)
	{
		auto const lock = std::lock_guard{ m_mutex };
		m_buffers.push_back(a_id);
	}

	void GlDeleteQueue::pushTexture(GraphicId const a_id)
	{
		auto const lock = std::lock_guard{ m_mutex };
		m_textures.push_back(a_id);
	}

	void GlDeleteQueue::pushFramebuffer(GraphicId const a_id)
	{
		auto const lock = std::lock_guard{ m_mutex };
		m_framebuffers.push_back(a_id);
	}

	void GlDeleteQueue::pushVertexArray(GraphicId const a_id)
	{
		auto const lock = std::lock_guard{ m_mutex };
		m_vertexArrays.push_back(a_id);
	}

	void GlDeleteQueue::pushProgram(GraphicId const a_id)
	{
		auto const lock = std::lock_guard{ m_mutex };
		m_programs.push_back(a_id);
	}

	void GlDeleteQueue::drain()
	{
		auto const lock = std::lock_guard{ m_mutex };

		glDeleteBuffers(mistd::isize(m_buffers), m_buffers.data());
		m_buffers.clear();

		glDeleteTextures(mistd::isize(m_textures), m_textures.data());
		m_textures.clear();

		glDeleteFramebuffers(mistd::isize(m_framebuffers), m_framebuffers.data());
		m_framebuffers.clear();

		glDeleteVertexArrays(mistd::isize(m_vertexArrays), m_vertexArrays.data());
		m_vertexArrays.clear();

		for (auto const id : m_programs)
		{
			glDeleteProgram(id);
		}
		m_programs.clear();
	}
}
