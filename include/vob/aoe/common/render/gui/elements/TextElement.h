#pragma once

#include <vob/aoe/common/render/gui/elements/AElement.h>
#include <vob/aoe/common/render/gui/Font.h>
#include <vob/aoe/common/render/Manager.h>
#include <vob/aoe/common/render/GraphicResourceHandle.h>
#include <vob/aoe/common/render/gui/GuiMesh.h>
#include <vob/aoe/common/render/gui/TextUtils.h>

namespace vob::aoe::common
{
	class TextElement
		: public AElement
	{
	public:
		#pragma region Constructors
		explicit TextElement(
			data::ADatabase& a_database
			, IGraphicResourceManager<GuiMesh>& a_guiMeshResourceManager
		)
			: m_font{ a_database }
			, m_mesh{ a_guiMeshResourceManager }
		{}
		#pragma endregion

		#pragma region Methods
		virtual void render(
			GuiShaderProgram const& a_shaderProgram
			, GuiRenderContext& a_renderContext
			, GuiTransform const a_transform
		) const override
		{
			assert(m_mesh->isReady());
			if (!m_font.isValid()
				|| !m_font->m_pages[0].isValid()
				|| !(*m_font->m_pages[0])->isReady())
			{
				return;
			}

			if (needsTextMeshUpdate(a_transform))
			{
				auto vertices = createTextMeshVertices(a_transform.m_size, m_text, m_size, *m_font);
				m_mesh->setVertices(vertices.data(), static_cast<u32>(vertices.size()));
			}

			glActiveTexture(GL_TEXTURE0 + 0);
			(*m_font->m_pages[0])->bind(GL_TEXTURE_2D);
			a_shaderProgram.setUniform(a_shaderProgram.m_renderType, c_renderTypeDistanceFieldFill);
			a_shaderProgram.setUniform(a_shaderProgram.m_elementPosition, a_transform.m_position);
			a_shaderProgram.setUniform(a_shaderProgram.m_elementSize, a_transform.m_size);
			m_mesh.resource()->render();
		}

		void setText(u8string a_text)
		{
			m_text = std::move(a_text);
			m_changed = true;
		}










		/****************************************************************/
		/****************************************************************/
		/****************************************************************/
		/****************************************************************/
		/****************************************************************/
		/****************************************************************/
		/****************************************************************/
		/*                  Polymorphic base for Clone                  */
		/****************************************************************/
		/****************************************************************/
		/****************************************************************/
		/****************************************************************/
		/****************************************************************/
		/****************************************************************/
		/****************************************************************/
























		void setFont(data::Handle<Font> a_font)
		{
			m_font = std::move(a_font);
			m_changed = true;
		}

		auto const& getFont() const
		{
			return m_font;
		}

		void setSize(float const a_size)
		{
			m_size = a_size;
			m_changed = true;
		}
		#pragma endregion

	private:
		#pragma region Attributes
		bool m_changed = false;
		u8string m_text;
		float m_size = 0;
		data::Handle<Font> m_font;
		mutable vec2 m_lastRenderedSize = { 0, 0 };
		GraphicResourceHandle<GuiMesh> m_mesh;
		#pragma endregion

		bool needsTextMeshUpdate(GuiTransform const& a_transform) const
		{
			return m_changed || a_transform.m_size != m_lastRenderedSize;
		}
	};
}
