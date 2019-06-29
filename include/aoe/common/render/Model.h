#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <aoe/core/data/Handle.h>
#include <aoe/common/render/Texture.h>

namespace aoe
{
	namespace common
	{
		struct Vertex
		{
			// Attributes
			glm::vec3 m_position;
			glm::vec3 m_normal;
			glm::vec2 m_textureCoordinates;

			// Constructors
			explicit Vertex(glm::vec3 a_position, glm::vec3 a_normal
				, glm::vec2 a_textureCoordinate)
				: m_position{ a_position }
				, m_normal{ a_normal }
				, m_textureCoordinates{ a_textureCoordinate }
			{}
		};

		class Mesh final
		{
		public:
			// Attributes
			std::pmr::vector<Vertex> m_vertices;
			std::pmr::vector<std::uint32_t> m_faces;

			std::uint32_t m_vertexArrayObject{};
			std::uint32_t m_vertexBufferObject{};
			std::uint32_t m_elementBufferObject{};

			// Constructor
			Mesh(std::pmr::vector<Vertex> a_vertices
				, std::pmr::vector<std::uint32_t> a_faces)
				: m_vertices{ std::move(a_vertices) }
				, m_faces{ std::move(a_faces) }
			{
				glGenVertexArrays(1, &m_vertexArrayObject);
				glGenBuffers(1, &m_vertexBufferObject);
				glGenBuffers(1, &m_elementBufferObject);

				glBindVertexArray(m_vertexArrayObject);

				glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
				glBufferData(GL_ARRAY_BUFFER
					, m_vertices.size() * sizeof(Vertex)
					, m_vertices.data(), GL_STATIC_DRAW);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBufferObject);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER
					, m_faces.size() * sizeof(std::uint32_t)
					, m_faces.data(), GL_STATIC_DRAW);

				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex)
					, reinterpret_cast<void*>(offsetof(Vertex, m_position)));

				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex)
					, reinterpret_cast<void*>(offsetof(Vertex, m_normal)));

				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex)
					, reinterpret_cast<void*>(offsetof(Vertex, m_textureCoordinates)));

				glBindVertexArray(0);
			}
		};

		struct Model final
			: public sta::ADynamicType
		{
			explicit Model(std::pmr::vector<Mesh> a_meshes)
				: m_meshes{ std::move(a_meshes) }
			{}

			// Attributes
			std::pmr::vector<Mesh> m_meshes;
		};

	}
}
