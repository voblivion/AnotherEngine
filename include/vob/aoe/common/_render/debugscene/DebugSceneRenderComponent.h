#pragma once

#include <vob/aoe/common/_render/GraphicResourceHandle.h>
#include <vob/aoe/common/_render/debugscene/DebugMesh.h>
#include <vob/aoe/common/_render/debugscene/DebugSceneShaderProgram.h>

#include <vob/aoe/core/data/ADatabase.h>

#include <vob/misc/std/ignorable_assert.h>


namespace vob::aoe::common
{
	struct DebugSceneRenderComponent final
	{
		// Attributes
		DebugMesh m_debugMesh{};
		std::shared_ptr<GraphicResourceHandle<DebugSceneShaderProgram> const> m_shaderProgram;
		IGraphicResourceManager<DebugSceneShaderProgram>& m_debugSceneShaderProgramResourceManager;

		// Constructor
		explicit DebugSceneRenderComponent(
			data::ADatabase& a_database
			, IGraphicResourceManager<DebugSceneShaderProgram>& a_debugSceneShaderProgramResourceManager
		)
			: m_debugSceneShaderProgramResourceManager{ a_debugSceneShaderProgramResourceManager  }
		{
			m_shaderProgram = a_database.find<GraphicResourceHandle<DebugSceneShaderProgram>>(6);
			ignorable_assert(m_shaderProgram != nullptr);
		}
	};
}