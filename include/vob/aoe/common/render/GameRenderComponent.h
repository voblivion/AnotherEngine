#pragma once

#include <array>

#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/common/opengl/resources/ShaderProgram.h>
#include <vob/aoe/common/opengl/Handle.h>
#include <vob/aoe/common/opengl/resources/RenderTexture.h>
#include "SimpleStaticMesh.h"
#include <vob/aoe/common/opengl/Manager.h>


namespace vob::aoe::common
{
	struct GameRenderComponent final
		: public ecs::AComponent
	{
		template <typename... Args>
		explicit GameRenderComponent(
			data::ADatabase& a_database
			, ogl::Manager<ogl::RenderTexture>& a_renderTextureManager
			, ogl::Manager<ogl::ShaderProgram>& a_shaderProgramResourceManager
			, ogl::Manager<ogl::SimpleStaticMesh>& a_simpleStaticMeshManager
			, ogl::Manager<ogl::StaticModel>& a_staticModelResourceManager
			, ogl::Manager<ogl::Texture>& a_textureManager
			, Args&&... a_args
		)
			: m_modelShader{ a_database }
			, m_uiShader{ a_database }
			, m_postProcessShader{ a_database }
			, m_gameRenderTexture{
				a_renderTextureManager
				, std::pmr::get_default_resource()
				, std::forward<Args>(a_args)...
			}
			, m_renderShape{ 
				a_simpleStaticMeshManager
				, std::pmr::get_default_resource()
			}
			, m_renderTextureManager{ a_renderTextureManager }
			, m_shaderProgramResourceManager{ a_shaderProgramResourceManager }
			, m_simpleStaticMeshManager{ a_simpleStaticMeshManager }
			, m_staticModelResourceManager{ a_staticModelResourceManager }
			, m_textureManager{ a_textureManager }
		{
			// TODO make it not hard coded
			m_postProcessShader.setId(21);
			ignorable_assert(m_postProcessShader.isValid());

			// TODO make it not hard coded
			m_modelShader.setId(2);
			ignorable_assert(m_modelShader.isValid());

			m_renderShape.resource().setData({
				ogl::SimpleVertex{{-1.0, -1.0, 0.0}, { 0.0, 0.0 }},
				ogl::SimpleVertex{{ 1.0, -1.0, 0.0}, { 1.0, 0.0 }},
				ogl::SimpleVertex{{-1.0,  1.0, 0.0}, { 0.0, 1.0 }},
				ogl::SimpleVertex{{ 1.0,  1.0, 0.0}, { 1.0, 1.0 }},
				ogl::SimpleVertex{{-1.0,  1.0, 0.0}, { 0.0, 1.0 }},
				ogl::SimpleVertex{{ 1.0, -1.0, 0.0}, { 1.0, 0.0 }}
			}, &m_gameRenderTexture.resource().m_texture);
		}
		GameRenderComponent(GameRenderComponent&&) = delete;

		// Model rendering
		data::Handle< ogl::Handle<ogl::ShaderProgram>> m_modelShader;

		// UI rendering
		data::Handle<ogl::Handle<ogl::ShaderProgram>> m_uiShader;

		// Post-process rendering
		data::Handle< ogl::Handle<ogl::ShaderProgram>> m_postProcessShader;
		ogl::Handle<ogl::RenderTexture> m_gameRenderTexture;
		ogl::Handle<ogl::SimpleStaticMesh> m_renderShape;

		// Resource Managers
		ogl::Manager<ogl::RenderTexture>& m_renderTextureManager;
		ogl::Manager<ogl::ShaderProgram>& m_shaderProgramResourceManager;
		ogl::Manager<ogl::SimpleStaticMesh>& m_simpleStaticMeshManager;
		ogl::Manager<ogl::StaticModel>& m_staticModelResourceManager;
		ogl::Manager<ogl::Texture>& m_textureManager;
	};
}
