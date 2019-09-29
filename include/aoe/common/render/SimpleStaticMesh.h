#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

#include <aoe/common/render/GlObjects.h>

namespace aoe::common
{
	struct SimpleVertex
	{
		glm::vec3 m_position;
		glm::vec2 m_texCoords;
	};

	class SimpleStaticMesh
	{
	public:
		void init(std::vector<SimpleVertex> const& a_vertices, gl::Texture* a_texture)
		{
			m_texture = a_texture;

			m_vao.tryCreate();
			gl::ResourceScopeUse<gl::VertexArray> t_vaoUse{ m_vao };

			m_vbo.tryCreate();
			gl::ResourceScopeUse<gl::Buffer> t_vboUse{ m_vbo };

			m_verticesCount = a_vertices.size();
			glBufferData(GL_ARRAY_BUFFER, a_vertices.size() * sizeof(SimpleVertex)
				, a_vertices.data(), GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex)
				, reinterpret_cast<void*>(offsetof(SimpleVertex, m_position)));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex)
				, reinterpret_cast<void*>(offsetof(SimpleVertex, m_texCoords)));
		}

		void draw() const
		{
			gl::ResourceScopeUse<gl::VertexArray> t_vaoUse{ m_vao };
			if (m_texture != nullptr)
			{
				glActiveTexture(GL_TEXTURE0);
				m_texture->startUsing();
			}
			glDrawArrays(GL_TRIANGLES, 0, m_verticesCount);
		}

		gl::VertexArray m_vao;
		gl::Buffer m_vbo;
		gl::Texture* m_texture{};
		std::size_t m_verticesCount{};
	};
}