#pragma once

#include <vob/aoe/common/render/gui/elements/AElement.h>
#include <vob/aoe/common/render/gui/GuiUtils.h>

#include <vob/aoe/common/render/gui/Value.h>

namespace vob::aoe::common
{
	using namespace literals;

	struct StandardElementStyle
    {
        // Attributes
		QuadValue m_outerCornerRadius{ 0_px };
		QuadValue m_innerCornerRadius{ 0_px };
		QuadValue m_borderWidth{ 0_px };
		QuadValue m_margin{ 0_px };
		QuadValue m_padding{ 0_px };
		QuadValue m_borderColor{ 0_px };
        std::shared_ptr<GraphicResourceHandle<Texture> const> m_borderTexture;
		QuadValue m_backgroundColor{ 0_px };
		std::shared_ptr<GraphicResourceHandle<Texture> const> m_backgroundTexture;
	};

	class VOB_AOE_API AStandardElement
		: public AElement
	{
	public:
		// Attributes
		glm::vec4 m_outerCornerRadius{ 0.0f };
		glm::vec4 m_innerCornerRadius{ 0.0f };
		glm::vec4 m_borderWidth{ 0.0f };
		glm::vec4 m_margin{ 0.0f }; // TODO should propagate surrounding elements margins, maybe?
		glm::vec4 m_padding{ 0.0f };
		glm::vec4 m_borderColor{ 0.0f };
		std::shared_ptr<GraphicResourceHandle<Texture> const> m_borderTexture;
		glm::vec4 m_backgroundColor{ 0.0f };
		std::shared_ptr<GraphicResourceHandle<Texture> const> m_backgroundTexture;

		// Methods
		virtual void render(
			GuiShaderProgram const& a_shaderProgram
			, GuiRenderContext& a_renderContext
			, GuiTransform a_transform
		) const final override
		{
			auto const topLeftMargin = glm::vec2{ m_margin.x, m_margin.y };
			auto const bottomRightMargin = glm::vec2{ m_margin.z, m_margin.w };
			auto const topLeftBorderWidth = glm::vec2{ m_borderWidth.x, m_borderWidth.y };
			auto const bottomRightBorderWidth = glm::vec2{ m_borderWidth.z, m_borderWidth.w };
			auto const topLeftPadding = glm::vec2{ m_padding.x, m_padding.y };
			auto const bottomRightPadding = glm::vec2{ m_padding.z, m_padding.w };

			auto const borderQuadPosition = a_transform.m_position + topLeftMargin;
			auto const borderQuadSize = a_transform.m_size - topLeftMargin - bottomRightMargin;

			a_shaderProgram.setRenderType(GuiRenderType::QuadStroke);
			a_shaderProgram.setOuterCornerRadius(m_outerCornerRadius);
			a_shaderProgram.setInnerCornerRadius(m_innerCornerRadius);
			a_shaderProgram.setStrokeWidth(m_borderWidth);
			a_shaderProgram.setElementPosition(borderQuadPosition);
			a_shaderProgram.setElementSize(borderQuadSize);
			a_shaderProgram.setColor(m_borderColor);
			a_renderContext.m_quad->render();

			auto const insideQuadPosition = borderQuadPosition + topLeftBorderWidth;
			auto const insideQuadSize = borderQuadSize - topLeftBorderWidth - bottomRightBorderWidth;

			a_shaderProgram.setRenderType(GuiRenderType::QuadFill);
			a_shaderProgram.setOuterCornerRadius(m_innerCornerRadius);
			a_shaderProgram.setElementPosition(insideQuadPosition);
			a_shaderProgram.setElementSize(insideQuadSize);
			a_shaderProgram.setColor(m_backgroundColor);
			a_renderContext.m_quad->render();

			auto const contentQuadPosition = insideQuadPosition + topLeftPadding;
			auto const contentQuadSize = insideQuadSize - topLeftPadding - bottomRightPadding;
			renderContent(a_shaderProgram, a_renderContext, GuiTransform{ contentQuadPosition, contentQuadSize });
		}

        template <typename VisitorType, typename Self>
		static bool accept(VisitorType& a_visitor, Self& a_this)
        {
            a_visitor.visit(misvi::nvp("Outer Corner Radius", a_this.m_outerCornerRadius));
            a_visitor.visit(misvi::nvp("Inner Corner Radius", a_this.m_innerCornerRadius));
            a_visitor.visit(misvi::nvp("Border Width", a_this.m_borderWidth));
            a_visitor.visit(misvi::nvp("Margin", a_this.m_margin));
            a_visitor.visit(misvi::nvp("Padding", a_this.m_padding));
            a_visitor.visit(misvi::nvp("Border Color", a_this.m_borderColor));
            a_visitor.visit(misvi::nvp("Border Texture", a_this.m_borderTexture));
            a_visitor.visit(misvi::nvp("Background Color", a_this.m_backgroundColor));
            a_visitor.visit(misvi::nvp("Background Texture", a_this.m_backgroundTexture));
			return true;
		}
	protected:
		// Methods
		virtual void renderContent(
			GuiShaderProgram const& a_shaderProgram
			, GuiRenderContext& a_renderContext
			, GuiTransform a_transform
		) const = 0;
	};
}

