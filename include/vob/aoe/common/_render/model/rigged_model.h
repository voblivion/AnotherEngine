#pragma once
#include <vob/aoe/api.h>

#include <glm/glm.hpp>
#include <vector>
#include <GL/glew.h>
#include <vob/aoe/common/_render/material.h>
#include <vob/aoe/common/_render/model/model_shader_program.h>
#include <vob/aoe/common/_render/primitive.h>

namespace vob::aoegl
{
	struct rigged_vertex
	{
		glm::vec3 m_position;
		glm::vec3 m_normal;
		glm::vec2 m_texCoords;
		glm::vec3 m_tangent;
		glm::u8vec4 m_boneIndices;
		glm::vec4 m_boneWeights;
	};

	struct rigged_mesh_data
	{
		using vertex = rigged_vertex;

		std::vector<vertex> m_vertices;
		std::vector<triangle> m_triangles;
	};

	struct rigged_model_data
	{
		std::vector<rigged_mesh_data> m_meshes;
	};

	struct rigged_mesh
	{
		graphic_object_id m_vao VOB_AOERN_INIT_GRAPHIC_OBJECT_ID;
		graphic_object_id m_vbo VOB_AOERN_INIT_GRAPHIC_OBJECT_ID;
		graphic_object_id m_ebo VOB_AOERN_INIT_GRAPHIC_OBJECT_ID;
		graphic_size_int m_triangleVertexCount;

		explicit rigged_mesh(rigged_mesh_data const& a_data)
			: m_triangleVertexCount{ static_cast<graphic_size_int>(a_data.m_triangles.size() * 3) }
		{
			using vertex = rigged_mesh_data::vertex;

			glGenVertexArrays(1, &m_vao);
			glBindVertexArray(m_vao);

			glGenBuffers(1, &m_vbo);
			glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
			glBufferData(
				GL_ARRAY_BUFFER
				, a_data.m_vertices.size() * sizeof(vertex)
				, a_data.m_vertices.data()
				, GL_STATIC_DRAW);

			glGenBuffers(1, &m_ebo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
			glBufferData(
				GL_ELEMENT_ARRAY_BUFFER
				, a_data.m_triangles.size() * sizeof(triangle)
				, a_data.m_triangles.data()
				, GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex)
				, reinterpret_cast<void*>(offsetof(vertex, m_position)));

			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex)
				, reinterpret_cast<void*>(offsetof(vertex, m_normal)));

			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex)
				, reinterpret_cast<void*>(offsetof(vertex, m_texCoords)));

			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex)
				, reinterpret_cast<void*>(offsetof(vertex, m_tangent)));

			glEnableVertexAttribArray(4);
			glVertexAttribIPointer(4, 3, GL_UNSIGNED_BYTE, sizeof(vertex)
				, reinterpret_cast<void*>(offsetof(vertex, m_boneIndices)));

			glEnableVertexAttribArray(5);
			glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(vertex)
				, reinterpret_cast<void*>(offsetof(vertex, m_boneWeights)));

			glBindVertexArray(0);
		}

		~rigged_mesh()
		{
			glDeleteBuffers(1, &m_ebo);
			glDeleteBuffers(1, &m_vbo);
			glDeleteVertexArrays(1, &m_vao);
		}

		void render() const
		{
			glBindVertexArray(m_vao);
			glDrawElements(GL_TRIANGLES, m_triangleVertexCount, GL_UNSIGNED_INT, nullptr);
		}
	};

	struct rigged_model
	{
		std::vector<rigged_mesh> m_meshes;

		explicit rigged_model(rigged_model_data const& a_data)
		{
			m_meshes.reserve(a_data.m_meshes.size());
			for (auto const& meshData : a_data.m_meshes)
			{
				m_meshes.emplace_back(meshData);
			}
		}
	};

	constexpr std::size_t k_maxBoneCount = 64;

	struct rig_pose
	{
		std::array<glm::mat4, k_maxBoneCount> m_boneTransforms;

		void set_uniform(model_shader_program const& a_shader)
		{
			glUniformMatrix4fv(
				a_shader.m_boneTransformsLocation
				, m_boneTransforms.size()
				, GL_FALSE
				, glm::value_ptr(m_boneTransforms[0]));
		}
	};
}

