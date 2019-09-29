#pragma once

#include <array>

#include <aoe/core/ecs/Component.h>
#include <aoe/common/render/ShaderProgram.h>
#include <aoe/common/render/RenderTexture.h>
#include "SimpleStaticMesh.h"


namespace aoe::common
{
	struct SceneRenderComponent final
		: public ecs::AComponent
	{
		template <typename... Args>
		explicit SceneRenderComponent(data::ADatabase& a_database, Args&&... a_args)
			: m_postProcessShader{ a_database }
		{
			m_renderTexture.init(std::forward<Args>(a_args)...);
			m_postProcessShader.setId(21);
			ignorableAssert(m_postProcessShader.isValid());

			m_renderShape.init({
				SimpleVertex{{-1.0, -1.0, 0.0}, { 0.0, 0.0 }},
				SimpleVertex{{ 1.0, -1.0, 0.0}, { 1.0, 0.0 }},
				SimpleVertex{{-1.0,  1.0, 0.0}, { 0.0, 1.0 }},
				SimpleVertex{{ 1.0,  1.0, 0.0}, { 1.0, 1.0 }},
				SimpleVertex{{-1.0,  1.0, 0.0}, { 0.0, 1.0 }},
				SimpleVertex{{ 1.0, -1.0, 0.0}, { 1.0, 0.0 }}
				}, &m_renderTexture.m_texture);
		}
		SceneRenderComponent(SceneRenderComponent&&) = delete;

		data::Handle<common::ShaderProgram> m_postProcessShader;
		RenderTexture m_renderTexture;
		SimpleStaticMesh m_renderShape;
	};
}
