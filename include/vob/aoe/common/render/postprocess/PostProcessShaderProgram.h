#pragma once
#include <vob/aoe/api.h>

#include <vob/aoe/common/render/resources/ShaderProgram.h>
#include <vob/aoe/common/render/OpenGl.h>


namespace vob::aoe::common
{
	class PostProcessShaderProgram
		: public ShaderProgram
	{
	public:
		auto getWindowSizeLocation() const
		{
			return m_windowSize;
		}

		auto getPostProcessTypeLocation() const
		{
			return m_postProcessType;
		}

		auto getBrightnessLocation() const
		{
			return m_brightness;
		}

		auto getSaturationLocation() const
		{
			return m_saturation;
		}

		void create() const
		{
			ShaderProgram::create();
			if (!isReady())
			{
				return;
			}

			m_windowSize = getUniformLocation("u_windowSize");
			m_postProcessType = getUniformLocation("u_postProcessType");
			m_brightness = getUniformLocation("u_brightness");
			m_saturation = getUniformLocation("u_saturation");
		}

	private:
		mutable UniformLocation m_windowSize = 0;
		mutable UniformLocation m_postProcessType = 0;
		mutable UniformLocation m_brightness = 0;
		mutable UniformLocation m_saturation = 0;
	};
}
