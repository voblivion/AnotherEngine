#pragma once

#include <vob/aoe/common/render/gui/elements/AElement.h>
#include <vob/aoe/common/render/gui/GuiUtils.h>


namespace vob::aoe::common
{
	class AStandardElement
		: public AElement
	{
	public:
		// Attributes
		vec4 m_outerCornerRadius{ 0.0f };
		vec4 m_innerCornerRadius{ 0.0f };
		vec4 m_borderWidth{ 0.0f };
		vec4 m_margin{ 0.0f }; // TODO should propagate surrounding elements margins, maybe?
		vec4 m_padding{ 0.0f };
		vec4 m_borderColor{ 1.0f };
		data::Handle<GraphicResourceHandle<Texture>> m_borderTexture;
		vec4 m_backgroundColor{ 1.0f };
		data::Handle<GraphicResourceHandle<Texture>> m_backgroundTexture;

		explicit AStandardElement(data::ADatabase& a_database)
			: m_borderTexture{ a_database }
			, m_backgroundTexture{ a_database }
		{}

		// Methods
		virtual void render(
			GuiShaderProgram const& a_shaderProgram
			, GuiRenderContext& a_renderContext
			, GuiTransform a_transform
		) const final override
		{
			auto const topLeftMargin = vec2{ m_margin.x, m_margin.y };
			auto const bottomRightMargin = vec2{ m_margin.z, m_margin.w };
			auto const topLeftBorderWidth = vec2{ m_borderWidth.x, m_borderWidth.y };
			auto const bottomRightBorderWidth = vec2{ m_borderWidth.z, m_borderWidth.w };
			auto const topLeftPadding = vec2{ m_padding.x, m_padding.y };
			auto const bottomRightPadding = vec2{ m_padding.z, m_padding.w };

			auto const borderQuadPosition = a_transform.m_position + topLeftMargin;
			auto const borderQuadSize = a_transform.m_size - topLeftMargin - bottomRightMargin;

			a_shaderProgram.setRenderType(GuiRenderType::QuadStroke);
			a_shaderProgram.setOuterCornerRadius(m_outerCornerRadius);
			a_shaderProgram.setInnerCornerRadius(m_innerCornerRadius);
			a_shaderProgram.setStrokeWidth(m_borderWidth);
			a_shaderProgram.setElementPosition(borderQuadPosition);
			a_shaderProgram.setElementSize(borderQuadSize);
			a_renderContext.m_quad->render();

			auto const insideQuadPosition = borderQuadPosition + topLeftBorderWidth;
			auto const insideQuadSize = borderQuadSize - topLeftBorderWidth - bottomRightBorderWidth;

			a_shaderProgram.setRenderType(GuiRenderType::QuadFill);
			a_shaderProgram.setOuterCornerRadius(m_innerCornerRadius);
			a_shaderProgram.setElementPosition(insideQuadPosition);
			a_shaderProgram.setElementPosition(insideQuadSize);
			a_renderContext.m_quad->render();

			auto const contentQuadPosition = insideQuadPosition + topLeftPadding;
			auto const contentQuadSize = insideQuadSize - topLeftPadding - bottomRightPadding;
			renderContent(a_shaderProgram, a_renderContext, GuiTransform{ contentQuadPosition, contentQuadSize });
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
