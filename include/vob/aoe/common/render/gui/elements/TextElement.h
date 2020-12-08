#pragma once

#include <vob/aoe/common/render/gui/elements/AElement.h>
#include <vob/aoe/common/render/gui/Font.h>
#include <vob/aoe/common/render/Manager.h>
#include <vob/aoe/common/render/GraphicResourceHandle.h>
#include <vob/aoe/common/render/gui/GuiMesh.h>
#include <vob/aoe/common/render/gui/TextUtils.h>
#include <vob/aoe/common/render/gui/elements/AStandardElement.h>

namespace vob::aoe::common
{
	class TextElement
		: public AStandardElement
	{
	public:
		#pragma region Constructors
		explicit TextElement(
			data::ADatabase& a_database
			, IGraphicResourceManager<GuiMesh>& a_guiMeshResourceManager
		)
			: AStandardElement{ a_database }
			, m_font{ a_database }
			, m_mesh{ a_guiMeshResourceManager }
		{}
		#pragma endregion

		#pragma region Methods
		virtual void renderContent(
			GuiShaderProgram const& a_shaderProgram
			, GuiRenderContext& a_renderContext
			, GuiTransform const a_transform
		) const override
		{
			if (!m_mesh->isReady()
				|| !m_font.isValid()
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
			a_shaderProgram.setRenderType(GuiRenderType::DistanceFieldFill);
			a_shaderProgram.setElementPosition(a_transform.m_position);
			a_shaderProgram.setElementSize(a_transform.m_size);
			m_mesh.resource()->render();
		}

		void setText(u8string a_text)
		{
			m_text = std::move(a_text);
			m_changed = true;
		}

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

	public: // TODO -> how to make accept friend ?
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

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ThisType>
	visitIfType<common::TextElement, ThisType> accept(VisitorType& a_visitor, ThisType& a_this)
	{
		if constexpr (!std::is_const_v<ThisType>)
		{
			a_this.m_changed = true;
		}
		a_visitor.visit(vis::makeNameValuePair("Text", a_this.m_text));
		a_visitor.visit(vis::makeNameValuePair("Size", a_this.m_size));
		a_visitor.visit(vis::makeNameValuePair("Font", a_this.m_font));
	}
}
