#pragma once

#include <array>

#include <vob/aoe/core/type/Primitive.h>

#include <vob/aoe/common/render/OpenGl.h>

namespace vob::aoe::common
{
	struct GuiVertex
	{
#pragma region Attributes
		vec2 m_position;
		vec2 m_textureCoords;
#pragma endregion
	};

	class GuiMesh
	{
	public:
#pragma region Methods
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

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(
				0
				, 2
				, GL_FLOAT
				, GL_FALSE
				, sizeof(GuiVertex)
				, reinterpret_cast<void*>(offsetof(GuiVertex, m_position))
			);

			glEnableVertexAttribArray(1);
			glVertexAttribPointer(
				1
				, 2
				, GL_FLOAT
				, GL_FALSE
				, sizeof(GuiVertex)
				, reinterpret_cast<void*>(offsetof(GuiVertex, m_textureCoords))
			);

			// By default, set GuiMesh to simplest quad
			std::array<GuiVertex, 6> vertices{
				GuiVertex{ { 0.0f, 0.0f }, { 0.0f, 0.0f } }
				, { { 0.0f, 1.0f }, { 0.0f, 1.0f } }
				, { { 1.0f, 1.0f }, { 1.0f, 1.0f } }
				, { { 1.0f, 1.0f }, { 1.0f, 1.0f } }
				, { { 1.0f, 0.0f }, { 1.0f, 0.0f } }
				, { { 0.0f, 0.0f }, { 0.0f, 0.0f } }
			};
			setVertices(vertices.data(), 6u);

			m_isReady = true;
		}

		void destroy() const
		{
			m_isReady = false;
			glDeleteBuffers(1, &m_vbo);
			glDeleteVertexArrays(1, &m_vao);
		}

		void setVertices(GuiVertex const* const a_first, u32 const a_count) const
		{
			m_vertexCount = a_count;
			glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
			glBufferData(GL_ARRAY_BUFFER, a_count * sizeof(GuiVertex), a_first, GL_STATIC_DRAW);
		}

		void render() const
		{
			glBindVertexArray(m_vao);
			glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
		}
#pragma endregion

	private:
#pragma region Attributes
		mutable bool m_isReady = false;
		mutable u32 m_vertexCount = 0;
		mutable GraphicObjectId m_vao INIT_GRAPHIC_OBJECT;
		mutable GraphicObjectId m_vbo INIT_GRAPHIC_OBJECT;
#pragma endregion
	};
}
