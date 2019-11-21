#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

#include <vob/aoe/common/opengl/resources/Object.h>

namespace vob::aoe::ogl
{
	struct SimpleVertex
	{
		glm::vec3 m_position;
		glm::vec2 m_texCoords;
	};

	class SimpleStaticMesh
	{
	public:
		void setData(std::vector<SimpleVertex> a_vertices, TextureObject* a_texture)
		{
			m_vertices = std::move(a_vertices);
			m_texture = a_texture;
		}

		void update()
		{
			glBindVertexArray(m_vao.m_id);
			glBindBuffer(GL_ARRAY_BUFFER, m_vbo.m_id);

			glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(SimpleVertex)
				, m_vertices.data(), GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex)
				, reinterpret_cast<void*>(offsetof(SimpleVertex, m_position)));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex)
				, reinterpret_cast<void*>(offsetof(SimpleVertex, m_texCoords)));
		}

		void create() const
		{
			m_vao.create();
			m_vbo.create();
		}

		bool isReady() const
		{
			return m_vao.isReady();
		}

		void destroy() const
		{
			m_vbo.destroy();
			m_vao.destroy();
		}

		void draw() const
		{
			glBindVertexArray(m_vao.m_id);
			if (m_texture != nullptr)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, m_texture->m_id);
			}
			glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));
		}

		VertexArrayObject m_vao;
		BufferObject m_vbo;
		std::vector<SimpleVertex> m_vertices;
		TextureObject* m_texture{};
	};
}