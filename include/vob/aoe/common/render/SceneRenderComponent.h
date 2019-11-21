#pragma once

#include <array>

#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/common/render/ShaderProgram.h>
#include <vob/aoe/common/render/RenderTexture.h>
#include "SimpleStaticMesh.h"


namespace vob::aoe::common
{
	struct GameRenderComponent final
		: public ecs::AComponent
	{
		template <typename... Args>
		explicit GameRenderComponent(data::ADatabase& a_database, Args&&... a_args)
			: m_postProcessShader{ a_database }
		{
			m_sceneRenderTexture.init(std::forward<Args>(a_args)...);
			m_postProcessShader.setId(21);
			ignorableAssert(m_postProcessShader.isValid());

			m_renderShape.init({
				SimpleVertex{{-1.0, -1.0, 0.0}, { 0.0, 0.0 }},
				SimpleVertex{{ 1.0, -1.0, 0.0}, { 1.0, 0.0 }},
				SimpleVertex{{-1.0,  1.0, 0.0}, { 0.0, 1.0 }},
				SimpleVertex{{ 1.0,  1.0, 0.0}, { 1.0, 1.0 }},
				SimpleVertex{{-1.0,  1.0, 0.0}, { 0.0, 1.0 }},
				SimpleVertex{{ 1.0, -1.0, 0.0}, { 1.0, 0.0 }}
				}, &m_sceneRenderTexture.m_texture);
		}
		GameRenderComponent(GameRenderComponent&&) = delete;

		data::Handle<common::ShaderProgram> m_postProcessShader;
		RenderTexture m_sceneRenderTexture;
		SimpleStaticMesh m_renderShape;
	};
}
