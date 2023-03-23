#pragma once

#include <vob/aoe/core/data/ADatabase.h>
#include <vob/aoe/common/_render/GraphicResourceHandle.h>
#include <vob/aoe/common/_render/Manager.h>
#include <vob/aoe/common/_render/model/model_shader_program.h>
#include <vob/aoe/common/_render/model/static_model.h>
#include <vob/aoe/common/_render/resources/RenderTexture.h>

#include <vob/misc/std/ignorable_assert.h>


namespace vob::aoe::common
{
	struct ModelRenderComponent final
	{
		// Attributes
		glm::vec3 m_ambientColor{ 0.25f, 0.25f, 0.25f };
		std::shared_ptr<GraphicResourceHandle<aoegl::model_shader_program> const> m_shaderProgram;
		// GraphicResourceHandle<RenderTexture> m_renderTexture;

		IGraphicResourceManager<Texture>& m_textureResourceManager;
		IGraphicResourceManager<RenderTexture>& m_renderTextureResourceManager;
		IGraphicResourceManager<static_model>& m_staticModelResourceManager;
		IGraphicResourceManager<aoegl::model_shader_program>& m_modelShaderProgramResourceManager;

		// Constructors
		explicit ModelRenderComponent(
			data::ADatabase& a_database
			, IGraphicResourceManager<Texture>& a_textureResourceManager
			, IGraphicResourceManager<RenderTexture>& a_renderTextureResourceManager
			, IGraphicResourceManager<static_model>& a_staticModelResourceManager
			, IGraphicResourceManager<aoegl::model_shader_program>& a_modelShaderProgramResourceManager
			, glm::ivec2 a_renderTextureSize
		)
			: m_textureResourceManager{ a_textureResourceManager }
			, m_renderTextureResourceManager{ a_renderTextureResourceManager }
			, m_staticModelResourceManager{ a_staticModelResourceManager }
			, m_modelShaderProgramResourceManager{ a_modelShaderProgramResourceManager }
		{
			m_shaderProgram = a_database.find<GraphicResourceHandle<aoegl::model_shader_program>>(1);
			ignorable_assert(m_shaderProgram != nullptr);
		}
	};
}
