#pragma once

#include <vob/aoe/common/render/GraphicResourceHandle.h>
#include <vob/aoe/common/render/postprocess/PostProcessShaderProgram.h>
#include <vob/aoe/common/render/postprocess/PostProcessQuad.h>

#include <vob/aoe/core/data/ADatabase.h>

#include <vob/misc/std/ignorable_assert.h>


namespace vob::aoe::common
{
	struct PostProcessRenderComponent final
	{
		// Attributes
		std::shared_ptr<GraphicResourceHandle<PostProcessShaderProgram> const> m_shaderProgram;
		PostProcessQuad m_quad;
		IGraphicResourceManager<PostProcessShaderProgram>& m_shaderProgramResourceManager;

		// Constructor
		explicit PostProcessRenderComponent(
			data::ADatabase& a_database
			, IGraphicResourceManager<PostProcessShaderProgram>& a_shaderProgramResourceManager
		)
			: m_quad{}
			, m_shaderProgramResourceManager{ a_shaderProgramResourceManager }
		{
			m_shaderProgram = a_database.find<GraphicResourceHandle<PostProcessShaderProgram>>(7);
			ignorable_assert(m_shaderProgram != nullptr);
		}
	};
}
