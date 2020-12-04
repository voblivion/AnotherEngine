#pragma once

#include <vob/aoe/common/render/gui/elements/AElement.h>
#include <vob/aoe/common/render/gui/GuiUtils.h>


namespace vob::aoe::common
{
	class StandardElement
		: public AElement
	{
	public:
		// Methods
		void render(GuiShaderProgram const& a_shaderProgram, GuiRenderContext& a_renderContext, GuiTransform a_transform) const override
		{
			auto const borderQuadPosition = a_transform.m_position + vec2{ m_margin };
			auto const borderQuadSize = a_transform.m_size - vec2{ m_margin } -vec2{ m_margin.z, m_margin.w };

			a_shaderProgram.setUniform(a_shaderProgram.m_renderType, c_renderTypeQuadStroke);
			a_shaderProgram.setUniform(a_shaderProgram.m_outerCornerRadius, m_outerCornerRadius);
			a_shaderProgram.setUniform(a_shaderProgram.m_innerCornerRadius, m_innerCornerRadius);
			a_shaderProgram.setUniform(a_shaderProgram.m_strokeWidth, m_borderWidth);
			a_shaderProgram.setUniform(a_shaderProgram.m_elementPosition, borderQuadPosition);
			a_shaderProgram.setUniform(a_shaderProgram.m_elementSize, borderQuadSize);
			a_renderContext.m_quad->render();

			auto const insideQuadPosition = borderQuadPosition + vec2{ m_borderWidth };
			auto const insideQuadSize = borderQuadSize - vec2{ m_borderWidth } - vec2{ m_borderWidth.z, m_borderWidth.w };

			a_shaderProgram.setUniform(a_shaderProgram.m_renderType, c_renderTypeQuadFill);
			a_shaderProgram.setUniform(a_shaderProgram.m_outerCornerRadius, m_innerCornerRadius);
			a_shaderProgram.setUniform(a_shaderProgram.m_elementPosition, insideQuadPosition);
			a_shaderProgram.setUniform(a_shaderProgram.m_elementSize, insideQuadSize);
			a_renderContext.m_quad->render();

			auto const contentQuadPosition = insideQuadPosition + vec2{ m_padding };
			auto const contentQuadSize = insideQuadSize - vec2{ m_padding } - vec2{ m_padding.z, m_padding.w };
			renderContent(a_shaderProgram, a_renderContext, GuiTransform{ contentQuadPosition, contentQuadSize });
		}

	protected:
		// Methods
		virtual void renderContent(GuiShaderProgram const& a_shaderProgram, GuiRenderContext& a_renderContext, GuiTransform a_transform) const = 0;

	public: // TODO
		// Attributes
		vec4 m_outerCornerRadius{};
		vec4 m_innerCornerRadius{};
		vec4 m_borderWidth{};
		vec4 m_margin{};
		vec4 m_padding{};
	};
}
