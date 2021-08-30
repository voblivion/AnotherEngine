#pragma once

#include <vob/sta/unicode.h>

#include <vob/aoe/api.h>

#include <vob/aoe/common/render/gui/elements/AElement.h>
#include <vob/aoe/common/render/gui/text/Font.h>
#include <vob/aoe/common/render/Manager.h>
#include <vob/aoe/common/render/GraphicResourceHandle.h>
#include <vob/aoe/common/render/gui/GuiMesh.h>
#include <vob/aoe/common/render/gui/text/TextUtils.h>
#include <vob/aoe/common/render/gui/elements/AStandardElement.h>

namespace vob::aoe::common
{
	class VOB_AOE_API TextElement
		: public AStandardElement
	{
	public:
		#pragma region Constructors
		explicit TextElement(IGraphicResourceManager<GuiMesh>& a_guiMeshResourceManager)
			: m_preSelectionMesh{ a_guiMeshResourceManager }
			, m_selectionMesh{ a_guiMeshResourceManager }
			, m_postSelectionMesh{ a_guiMeshResourceManager }
		{}
		#pragma endregion

		#pragma region Methods
		bool onEvent(WindowEvent const& a_event, GuiTransform a_transform) override;

		void renderContent(
			GuiShaderProgram const& a_shaderProgram
			, GuiRenderContext& a_renderContext
			, GuiTransform const a_transform
		) const override;

		void setText(sta::utf8_string a_text);

        auto const& getFont() const;
		void setFont(std::shared_ptr<aoe::common::Font const> a_font);

		void setSize(float const a_size);
		#pragma endregion

        template <typename VisitorType, typename ThisType>
        static void accept(VisitorType& a_visitor, ThisType& a_this)
        {
			AStandardElement::accept(a_visitor, a_this);

            std::string text;
            a_visitor.visit(vis::makeNameValuePair("Text", text));
            a_this.m_text.assign(text);
            a_visitor.visit(vis::makeNameValuePair("Size", a_this.m_size));
            a_visitor.visit(vis::makeNameValuePair("Line Height", a_this.m_lineHeight));
            a_visitor.visit(vis::makeNameValuePair("Font", a_this.m_font));
            a_visitor.visit(vis::makeNameValuePair("Color", a_this.m_color));
            a_this.m_hasChanged = true;
        }

	protected:
		#pragma region Attributes
		mutable bool m_hasChanged = false;
		sta::utf8_string m_text;
        float m_size = 12.0f;
		std::optional<float> m_lineHeight;
		std::shared_ptr<Font const> m_font;
		vec4 m_color{ 1.0f };
		mutable vec2 m_lastRenderedSize = { 0, 0 };
		GraphicResourceHandle<GuiMesh> m_preSelectionMesh;
		GraphicResourceHandle<GuiMesh> m_selectionMesh;
		GraphicResourceHandle<GuiMesh> m_postSelectionMesh;
		std::size_t m_selectionStart = 0;
		std::size_t m_selectionEnd = 0;
		vec2 m_mousePos = {};
		bool m_isSelecting = false;
		#pragma endregion

		bool needsTextMeshUpdate(GuiTransform const& a_transform) const
		{
			return m_hasChanged || a_transform.m_size != m_lastRenderedSize;
		}
	};
}

