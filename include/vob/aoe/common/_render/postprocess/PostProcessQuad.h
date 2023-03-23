#pragma once

#include <array>

#include <vob/aoe/common/_render/OpenGl.h>


namespace vob::aoe::common
{
	struct PostProcessVertex
	{
		glm::vec2 m_position;
		glm::vec2 m_textureCoordinates;
	};

	class PostProcessQuad
	{
	public:
		// Methods
		bool isReady() const
		{
			return m_isReady;
		}
		void create() const
		{
			glGenVertexArrays(1, &m_vao);
			glBindVertexArray(m_vao);

			glGenBuffers(1, &m_vbo);
			glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
			std::array<PostProcessVertex, 6> vertices{
				PostProcessVertex{ {-1.0f, -1.0f}, {0.0f, 0.0f} }
				, { {1.0f, -1.0f}, {1.0f, 0.0f} }
				, { {1.0f, 1.0f}, {1.0f, 1.0f} }
				, { {1.0f, 1.0f}, {1.0f, 1.0f} }
				, { {-1.0f, 1.0f}, {0.0f, 1.0f} }
				, { {-1.0f, -1.0f}, {0.0f, 0.0f} }
			};
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(
				0
				, 2
				, GL_FLOAT
				, GL_FALSE
				, sizeof(PostProcessVertex)
				, reinterpret_cast<void*>(offsetof(PostProcessVertex, m_position))
			);

			glEnableVertexAttribArray(1);
			glVertexAttribPointer(
				1
				, 2
				, GL_FLOAT
				, GL_FALSE
				, sizeof(PostProcessVertex)
				, reinterpret_cast<void*>(offsetof(PostProcessVertex, m_textureCoordinates))
			);
			m_isReady = true;
		}
		void render() const
		{
			glBindVertexArray(m_vao);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		void destroy() const
		{
			m_isReady = false;
			glDeleteBuffers(1, &m_vbo);
			glDeleteVertexArrays(1, &m_vao);
		}

	private:
		// Attributes
		mutable bool m_isReady = false;
		mutable GraphicObjectId m_vao;
		mutable GraphicObjectId m_vbo;
	};
}