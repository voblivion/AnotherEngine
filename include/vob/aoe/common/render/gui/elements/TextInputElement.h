#pragma once

#include <vob/aoe/common/render/gui/elements/TextElement.h>
#include <vob/aoe/common/time/Chrono.h>

namespace vob::aoe::common
{
	class VOB_AOE_API TextInputElement
		: public TextElement
	{
	public:
		explicit TextInputElement(
			IGraphicResourceManager<GuiMesh>& a_guiMeshResourceManager
		)
			: TextElement{ a_guiMeshResourceManager }
		{}

		bool onEvent(WindowEvent const& a_event, GuiTransform a_transform) override;

		void renderContent(
			GuiShaderProgram const& a_shaderProgram
			, GuiRenderContext& a_renderContext
			, GuiTransform a_transform) const override;

        template <typename VisitorType, typename ThisType>
		static bool accept(VisitorType& a_visitor, ThisType& a_this)
		{
			TextElement::accept(a_visitor, a_this);
			return true;
		}

	private:
		void insert(char32_t a_unicode);
		void erase(std::size_t a_count);

		size_t prevWordStart() const;
		size_t nextWordStart() const;

		mutable bool m_needsCursorUpdate = true;
		mutable std::pair<glm::vec2, glm::vec2> m_cursorTransform = {};
		mutable TimePoint m_lastCursorUpdateTime = {};
		glm::ivec2 m_mousePosition = {};
		bool m_isMouseLeftDown = false;
	};
}

