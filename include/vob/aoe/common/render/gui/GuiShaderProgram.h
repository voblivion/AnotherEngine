#pragma once
#include <vob/aoe/common/render/resources/ShaderProgram.h>
#include <vob/aoe/common/render/resources/SceneShaderProgram.h>

namespace vob::aoe::common
{
	constexpr std::uint32_t c_renderTypeQuadFill = 0;
	constexpr std::uint32_t c_renderTypeQuadStroke = 1;
	constexpr std::uint32_t c_renderTypeDistanceFieldFill = 2;
	constexpr std::uint32_t c_renderTypeDistanceFieldStroke = 3;

	class GuiShaderProgram
		: public ShaderProgram
	{
	public:
		// Constructor
		explicit GuiShaderProgram(data::ADatabase& a_database)
			: ShaderProgram{ a_database }
		{}

		// Methods
		auto getViewSizeUniformLocation() const
		{
			return m_viewSize;
		}
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
			
		}

	public: // TODO
		// Attributes
		mutable UniformLocation m_viewSize = 0;
		mutable UniformLocation m_renderType = 0;
		mutable UniformLocation m_elementPosition = 0;
		mutable UniformLocation m_elementSize = 0;
		// RENDER_TYPE_QUAD
		mutable UniformLocation m_outerCornerRadius = 0;
		mutable UniformLocation m_innerCornerRadius = 0;
		mutable UniformLocation m_strokeWidth = 0;
	};
}
