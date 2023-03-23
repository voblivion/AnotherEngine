#pragma once
#include <vob/aoe/api.h>

#include <glm/glm.hpp>
#include <vector>
#include <GL/glew.h>
#include <vob/aoe/common/_render/OpenGl.h>
#include <vob/aoe/common/_render/primitive.h>
#include <vob/aoe/common/_render/material.h>

namespace vob::aoe::common
{
	struct static_vertex
	{
		glm::vec3 m_position;
		glm::vec3 m_normal;
		glm::vec2 m_texCoords;
		glm::vec3 m_tangent;
	};

	class VOB_AOE_API StaticMesh final
	{
	public:
		// Constructors / Destructor
		StaticMesh(
			std::vector<static_vertex> a_vertices
			, std::vector<triangle> a_triangles
			, std::size_t const a_materialIndex
		)
			: m_vertices{ std::move(a_vertices) }
			, m_triangles{ std::move(a_triangles) }
			, m_materialIndex{ a_materialIndex }
		{
		}

		// Accessors
		auto getMaterialIndex() const
		{
			return m_materialIndex;
		}

		// Methods
		void render() const
		{
			glBindVertexArray(m_vao);
			glDrawElements(
				GL_TRIANGLES
				, static_cast<GLsizei>(m_triangles.size() * 3)
				, GL_UNSIGNED_INT
				, nullptr
			);
		}
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
			glBufferData(
				GL_ARRAY_BUFFER
				, m_vertices.size() * sizeof(static_vertex)
				, m_vertices.data()
				, GL_STATIC_DRAW
			);

			glGenBuffers(1, &m_ebo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
			glBufferData(
				GL_ELEMENT_ARRAY_BUFFER
				, m_triangles.size() * sizeof(triangle)
				, m_triangles.data()
				, GL_STATIC_DRAW
			);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(static_vertex)
				, reinterpret_cast<void*>(offsetof(static_vertex, m_position)));

			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(static_vertex)
				, reinterpret_cast<void*>(offsetof(static_vertex, m_normal)));

			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(static_vertex)
				, reinterpret_cast<void*>(offsetof(static_vertex, m_texCoords)));

			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(static_vertex)
				, reinterpret_cast<void*>(offsetof(static_vertex, m_tangent)));

			glBindVertexArray(0);
			m_isReady = true;
		}
		void destroy() const
		{
			m_isReady = false;
			glDeleteBuffers(1, &m_ebo);
			glDeleteBuffers(1, &m_vbo);
			glDeleteVertexArrays(1, &m_vao);
		}

		std::vector<static_vertex> const& getVertices() const { return m_vertices; }
		std::vector<triangle> const& getTriangles() const { return m_triangles; }

	private:
		// Attributes
		std::vector<static_vertex> m_vertices;
		std::vector<triangle> m_triangles;
		std::size_t m_materialIndex;

		mutable bool m_isReady = false;
		mutable GraphicObjectId m_vao INIT_GRAPHIC_OBJECT;
		mutable GraphicObjectId m_vbo INIT_GRAPHIC_OBJECT;
		mutable GraphicObjectId m_ebo INIT_GRAPHIC_OBJECT;
	};

	struct VOB_AOE_API static_model
	{
		std::vector<StaticMesh> m_meshes;
		std::vector<old_material> m_materials;

		explicit static_model(std::vector<StaticMesh> a_meshes, std::vector<old_material> a_materials)
			: m_meshes{ std::move(a_meshes) }
			, m_materials{ std::move(a_materials) }
		{}

		void create() const
		{
			for (auto const& t_mesh : m_meshes)
			{
				t_mesh.create();
			}
		}

		bool isReady() const
		{
			return m_meshes.empty() || m_meshes[0].isReady();
		}

		void destroy() const
		{
			for (auto const& t_mesh : m_meshes)
			{
				t_mesh.destroy();
			}
		}
	};
}
