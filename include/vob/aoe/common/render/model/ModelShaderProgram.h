#pragma once
#include <vob/aoe/api.h>

#include <vob/aoe/common/render/resources/SceneShaderProgram.h>

namespace vob::aoe::common
{
	class ModelShaderProgram
		: public SceneShaderProgram
	{
	public:
		// Methods
		auto getAmbientColorUniformLocation() const
		{
			return m_ambientColor;
		}

		auto getLightPositionUniformLocation() const
		{
			return m_lightPosition;
		}
		
		auto getModelUniformLocation() const
		{
			return m_model;
		}
		auto getModelNormalUniformLocation() const
		{
			return m_modelNormal;
		}
		void create() const
		{
			SceneShaderProgram::create();
			if (!isReady())
			{
				return;
			}

			m_ambientColor = getUniformLocation("u_ambientColor");
			m_lightPosition = getUniformLocation("u_lightPos");
			m_model = getUniformLocation("u_model");
			m_modelNormal = getUniformLocation("u_modelNormal");
		}

	private:
		// Attributes
		mutable UniformLocation m_ambientColor;
		mutable UniformLocation m_lightPosition;
		mutable UniformLocation m_model;
		mutable UniformLocation m_modelNormal;
		mutable UniformLocation m_materialDiffuse;
		mutable UniformLocation m_materialSpecular;
	};
}
