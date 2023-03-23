#pragma once
#include <vob/aoe/api.h>

#include <vob/aoe/common/_render/resources/SceneShaderProgram.h>

namespace vob::aoegl
{
	class model_shader_program
		: public aoe::common::SceneShaderProgram
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

		// Attributes
		mutable graphic_uniform_location m_ambientColor;
		mutable graphic_uniform_location m_lightPosition;
		mutable graphic_uniform_location m_model;
		mutable graphic_uniform_location m_modelNormal;
		mutable graphic_uniform_location m_materialDiffuse;
		mutable graphic_uniform_location m_materialSpecular;
		graphic_uniform_location m_boneTransformsLocation;

		graphic_enum m_albedoTexture = GL_TEXTURE0;
		graphic_enum m_normalTexture = GL_TEXTURE1;
		graphic_enum m_metallicRoughnessTexture = GL_TEXTURE2;
	};
}
