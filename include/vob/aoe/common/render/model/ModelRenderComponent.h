#pragma once

#include <vob/sta/ignorable_assert.h>

#include <vob/aoe/core/data/ADatabase.h>
#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/common/render/GraphicResourceHandle.h>
#include <vob/aoe/common/render/Manager.h>
#include <vob/aoe/common/render/model/ModelShaderProgram.h>
#include <vob/aoe/common/render/model/StaticModel.h>
#include <vob/aoe/common/render/resources/RenderTexture.h>

namespace vob::aoe::common
{
	struct ModelRenderComponent final
		: public ecs::AComponent
	{
		// Attributes
		glm::vec3 m_ambientColor{ 0.25f, 0.25f, 0.25f };
		std::shared_ptr<GraphicResourceHandle<ModelShaderProgram> const> m_shaderProgram;
		// GraphicResourceHandle<RenderTexture> m_renderTexture;

		IGraphicResourceManager<Texture>& m_textureResourceManager;
		IGraphicResourceManager<RenderTexture>& m_renderTextureResourceManager;
		IGraphicResourceManager<StaticModel>& m_staticModelResourceManager;
		IGraphicResourceManager<ModelShaderProgram>& m_modelShaderProgramResourceManager;

		// Constructors
		explicit ModelRenderComponent(
			data::ADatabase& a_database
			, IGraphicResourceManager<Texture>& a_textureResourceManager
			, IGraphicResourceManager<RenderTexture>& a_renderTextureResourceManager
			, IGraphicResourceManager<StaticModel>& a_staticModelResourceManager
			, IGraphicResourceManager<ModelShaderProgram>& a_modelShaderProgramResourceManager
			, glm::ivec2 a_renderTextureSize
		)
			: m_textureResourceManager{ a_textureResourceManager }
			, m_renderTextureResourceManager{ a_renderTextureResourceManager }
			, m_staticModelResourceManager{ a_staticModelResourceManager }
			, m_modelShaderProgramResourceManager{ a_modelShaderProgramResourceManager }
		{
			m_shaderProgram = a_database.find<GraphicResourceHandle<ModelShaderProgram>>(1);
			ignorable_assert(m_shaderProgram != nullptr);
		}
	};
}
