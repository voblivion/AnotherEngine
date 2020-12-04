#pragma once

#include <vob/sta/ignorable_assert.h>

#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/core/data/ADatabase.h>
#include <vob/aoe/core/data/Handle.h>

#include <vob/aoe/common/render/GraphicResourceHandle.h>
#include <vob/aoe/common/render/postprocess/PostProcessShaderProgram.h>
#include <vob/aoe/common/render/postprocess/PostProcessQuad.h>


namespace vob::aoe::common
{
	struct PostProcessRenderComponent final
		: public ecs::AComponent
	{
		// Attributes
		data::Handle<GraphicResourceHandle<PostProcessShaderProgram>> m_shaderProgram;
		PostProcessQuad m_quad;
		IGraphicResourceManager<PostProcessShaderProgram>& m_shaderProgramResourceManager;

		// Constructor
		explicit PostProcessRenderComponent(
			data::ADatabase& a_database
			, IGraphicResourceManager<PostProcessShaderProgram>& a_shaderProgramResourceManager
		)
			: m_shaderProgram{ a_database }
			, m_quad{}
			, m_shaderProgramResourceManager{ a_shaderProgramResourceManager }
		{
			m_shaderProgram.setId(7);
			ignorable_assert(m_shaderProgram.isValid());
		}
	};
}
