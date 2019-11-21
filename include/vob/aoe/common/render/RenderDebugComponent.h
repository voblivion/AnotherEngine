#pragma once
#include "vob/aoe/core/ecs/Component.h"
#include "Model.h"
#include <vector>
#include <GL/glew.h>
#include "ShaderProgram.h"
#include "Texture.h"
#include <cassert>

// TODO
#include "vob/aoe/core/utils/SimpleProfiler.h"


namespace vob::aoe::common
{
	struct DebugVertex
	{
		glm::vec3 m_position{ 0.0f };
		glm::vec4 m_color{ 0.0f };

		explicit DebugVertex() = default;
		explicit DebugVertex(glm::vec3 const& a_position
			, glm::vec4 const& a_color = glm::vec4{ 1.0f })
			: m_position{ a_position }
			, m_color{ a_color }
		{}

		explicit DebugVertex(glm::vec3 const& a_position
			, glm::vec3 const& a_color)
			: m_position{ a_position }
			, m_color{ a_color, 1.0f }
		{}
	};

	struct DebugMesh
	{
		// Attributes
		std::pmr::vector<DebugVertex> m_vertices;
		std::pmr::vector<std::uint32_t> m_lines;

		std::uint32_t m_vertexArrayObject{};
		std::uint32_t m_vertexBufferObject{};
		std::uint32_t m_elementBufferObject{};

		// Constructor
		DebugMesh()
		{
			glGenVertexArrays(1, &m_vertexArrayObject);
			glGenBuffers(1, &m_vertexBufferObject);
			glGenBuffers(1, &m_elementBufferObject);
		}

		void addLine(DebugVertex const& a_source, DebugVertex const& a_target)
		{
			m_vertices.emplace_back(a_source);
			m_vertices.emplace_back(a_target);
			m_lines.emplace_back(static_cast<std::uint32_t>(m_lines.size()));
			m_lines.emplace_back(static_cast<std::uint32_t>(m_lines.size()));
		}

		void reset()
		{
			m_vertices.clear();
			m_lines.clear();
		}

		void update()
		{
			glBindVertexArray(m_vertexArrayObject);

			glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
			glBufferData(GL_ARRAY_BUFFER
				, m_vertices.size() * sizeof(DebugVertex)
				, m_vertices.data(), GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBufferObject);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER
				, m_lines.size() * sizeof(std::uint32_t)
				, m_lines.data(), GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE
				, sizeof(DebugVertex)
				, reinterpret_cast<void*>(offsetof(DebugVertex, m_position)));

			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE
				, sizeof(DebugVertex)
				, reinterpret_cast<void*>(offsetof(DebugVertex, m_color)));

			glBindVertexArray(0);
		}
	};

	struct DebugRenderComponent final
		: public ecs::AComponent
	{
		explicit DebugRenderComponent(data::ADatabase& a_database)
			: m_modelShader{ a_database }
			, m_debugShader{ a_database }
		{
			m_modelShader.setId(2);
			ignorableAssert(m_modelShader.isValid());
			m_debugShader.setId(16);
			ignorableAssert(m_debugShader.isValid());
		}

		data::Handle<ShaderProgram> m_modelShader;
		data::Handle<ShaderProgram> m_debugShader;
		DebugMesh m_debugMesh;
	};
}
