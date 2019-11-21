#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <GL/glew.h>
#include <vob/aoe/core/data/Handle.h>
#include <vob/aoe/core/visitor/Aggregate.h>
#include <vob/aoe/common/opengl/resources/Object.h>
#include <vob/aoe/common/opengl/Manager.h>

namespace vob::aoe::ogl
{
	struct Vertex
	{
		glm::vec3 m_position;
		glm::vec3 m_normal;
		glm::vec2 m_textureCoordinates;
	};

	struct StaticMesh final
	{
		std::pmr::vector<Vertex> m_vertices;
		std::pmr::vector<std::uint32_t> m_faces;
		VertexArrayObject m_vao;
		VertexArrayObject m_vbo;
		VertexArrayObject m_ebo;

		StaticMesh(
			std::pmr::vector<Vertex> a_vertices
			, std::pmr::vector<std::uint32_t> a_faces
		)
			: m_vertices{ std::move(a_vertices) }
			, m_faces{ std::move(a_faces) }
		{}

		void create() const
		{
			m_vao.create();
			m_vbo.create();
			m_ebo.create();

			glBindVertexArray(m_vao.m_id);

#define VOB_AOE_OGL_CONTAINER_VIEW(container) \
	container.size() * sizeof(decltype(container)::value_type), container.data()

			glBindBuffer(GL_ARRAY_BUFFER, m_vbo.m_id);
			glBufferData(
				GL_ARRAY_BUFFER
				, VOB_AOE_OGL_CONTAINER_VIEW(m_vertices)
				, GL_STATIC_DRAW
			);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo.m_id);
			glBufferData(
				GL_ELEMENT_ARRAY_BUFFER
				, VOB_AOE_OGL_CONTAINER_VIEW(m_faces)
				, GL_STATIC_DRAW
			);

#undef VOB_AOE_OGL_CONTAINER_VIEW

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

		bool isReady() const
		{
			return m_vao.isReady();
		}

		void destroy() const
		{
			m_ebo.destroy();
			m_vbo.destroy();
			m_vao.destroy();
		}
	};

	struct StaticModel
	{
		std::pmr::vector<StaticMesh> m_meshes;

		explicit StaticModel(std::pmr::vector<StaticMesh> a_meshes)
			: m_meshes{ std::move(a_meshes) }
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
