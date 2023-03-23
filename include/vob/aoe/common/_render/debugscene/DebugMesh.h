#pragma once

#include <vector>

#include <vob/aoe/common/_render/OpenGl.h>
#include <vob/aoe/common/_render/primitive.h>


namespace vob::aoe::common
{
	struct DebugVertex
	{
		// Attributes
		glm::vec3 m_position{ 0.0f };
		glm::vec4 m_color{ 1.0f };

		// Constructors
		explicit DebugVertex() = default;
		explicit DebugVertex(glm::vec3 const& a_position, glm::vec4 const& a_color = glm::vec4{ 1.0f })
			: m_position{ a_position }
			, m_color{ a_color }
		{}

		explicit DebugVertex(glm::vec3 const& a_position, glm::vec3 const& a_color)
			: m_position{ a_position }
			, m_color{ a_color, 1.0f }
		{}
	};

	class DebugMesh
	{
	public:
		// Methods
		void addLine(DebugVertex const& a_source, DebugVertex const& a_target)
		{
			auto const v0 = static_cast<std::uint32_t>(m_vertices.size());
			m_vertices.emplace_back(a_source);
			auto const v1 = static_cast<std::uint32_t>(m_vertices.size());
			m_vertices.emplace_back(a_target);
			m_lines.emplace_back(line{ v0, v1 });
		}
		void reset()
		{
			m_vertices.clear();
			m_lines.clear();
		}
		void update() const
		{
			glBindVertexArray(m_vao);

			glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
			glBufferData(
				GL_ARRAY_BUFFER
				, m_vertices.size() * sizeof(DebugVertex)
				, m_vertices.data()
				, GL_STATIC_DRAW
			);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
			glBufferData(
				GL_ELEMENT_ARRAY_BUFFER
				, m_lines.size() * sizeof(line)
				, m_lines.data()
				, GL_STATIC_DRAW
			);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(
				0
				, 3
				, GL_FLOAT
				, GL_FALSE
				, sizeof(DebugVertex)
				, reinterpret_cast<void*>(offsetof(DebugVertex, m_position))
			);

			glEnableVertexAttribArray(1);
			glVertexAttribPointer(
				1
				, 3
				, GL_FLOAT
				, GL_FALSE
				, sizeof(DebugVertex)
				, reinterpret_cast<void*>(offsetof(DebugVertex, m_color))
			);
		}
		void render() const
		{
			glBindVertexArray(m_vao);
			glDrawElements(
				GL_LINES
				, static_cast<std::uint32_t>(m_lines.size()) * 2
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
			glGenBuffers(1, &m_vbo);
			glGenBuffers(1, &m_ebo);
			m_isReady = true;
		}
		void destroy() const
		{
			glDeleteBuffers(1, &m_ebo);
			glDeleteBuffers(1, &m_vbo);
			glDeleteVertexArrays(1, &m_vao);
			m_isReady = false;
		}

	private:
		// Attributes
		std::vector<DebugVertex> m_vertices;
		std::vector<line> m_lines;

		mutable bool m_isReady = false;
		mutable GraphicObjectId m_vao INIT_GRAPHIC_OBJECT;
		mutable GraphicObjectId m_vbo INIT_GRAPHIC_OBJECT;
		mutable GraphicObjectId m_ebo INIT_GRAPHIC_OBJECT;
	};
}
