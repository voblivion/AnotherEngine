#pragma once
#include <vob/aoe/common/render/resources/ShaderProgram.h>
#include <vob/aoe/common/render/resources/SceneShaderProgram.h>
#include <vob/aoe/core/type/Primitive.h>

namespace vob::aoe::common
{
	enum class GuiRenderType : u32
	{
		QuadFill = 0
		, QuadStroke = 1
		, DistanceFieldFill = 2
		, DistanceFieldStroke = 3
	};

	constexpr std::uint32_t c_renderTypeQuadFill = 0;
	constexpr std::uint32_t c_renderTypeQuadStroke = 1;
	constexpr std::uint32_t c_renderTypeDistanceFieldFill = 2;
	constexpr std::uint32_t c_renderTypeDistanceFieldStroke = 3;

	class GuiShaderProgram
		: public ShaderProgram
	{
	public:
		// Methods
		void create() const
		{
			ShaderProgram::create();
			if (!isReady())
			{
				return;
			}

			m_renderType = getUniformLocation("u_renderType");
			m_viewSize = getUniformLocation("u_viewSize");
			m_elementPosition = getUniformLocation("u_elementPosition");
			m_elementSize = getUniformLocation("u_elementSize");
			m_outerCornerRadius = getUniformLocation("u_outerCornerRadius");
			m_innerCornerRadius = getUniformLocation("u_innerCornerRadius");
			m_strokeWidth = getUniformLocation("u_strokeWidth");
			m_color = getUniformLocation("u_color");
		}

		void setViewSize(vec2 const& a_viewSize) const
		{
			setUniform(m_viewSize, a_viewSize);
		}

		void setRenderType(GuiRenderType a_renderType) const
		{
			setUniform(m_renderType, static_cast<u32>(a_renderType));
		}

		void setElementPosition(vec2 const& a_elementPosition) const
		{
			setUniform(m_elementPosition, a_elementPosition);
		}

		void setElementSize(vec2 const& a_elementSize) const
		{
			setUniform(m_elementSize, a_elementSize);
		}

		void setOuterCornerRadius(vec4 const& a_outerCornerRadius) const
		{
			setUniform(m_outerCornerRadius, a_outerCornerRadius);
		}

		void setInnerCornerRadius(vec4 const& a_innerCornerRadius) const
		{
			setUniform(m_innerCornerRadius, a_innerCornerRadius);
		}

		void setStrokeWidth(vec4 const& a_strokeWidth) const
		{
			setUniform(m_strokeWidth, a_strokeWidth);
		}

		void setColor(vec4 const& a_color) const
		{
			setUniform(m_color, a_color);
		}

	private:
		// Attributes
		mutable UniformLocation m_viewSize = 0;
		mutable UniformLocation m_renderType = 0;
		mutable UniformLocation m_elementPosition = 0;
		mutable UniformLocation m_elementSize = 0;
		mutable UniformLocation m_color = 0;
		// RENDER_TYPE_QUAD - TODO
		mutable UniformLocation m_outerCornerRadius = 0;
		mutable UniformLocation m_innerCornerRadius = 0;
		mutable UniformLocation m_strokeWidth = 0;
	};
}
