#pragma once
#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/core/data/ADatabase.h>
#include <vob/aoe/core/data/Handle.h>

#include <vob/aoe/common/render/GraphicResourceHandle.h>
#include <vob/aoe/common/render/debugscene/DebugMesh.h>
#include <vob/aoe/common/render/debugscene/DebugSceneShaderProgram.h>
#include <vob/sta/ignorable_assert.h>

namespace vob::aoe::common
{
	struct DebugSceneRenderComponent final
		: public ecs::AComponent
	{
		// Attributes
		DebugMesh m_debugMesh{};
		data::Handle<GraphicResourceHandle<DebugSceneShaderProgram>> m_shaderProgram;
		IGraphicResourceManager<DebugSceneShaderProgram>& m_debugSceneShaderProgramResourceManager;

		// Constructor
		explicit DebugSceneRenderComponent(
			data::ADatabase& a_database
			, IGraphicResourceManager<DebugSceneShaderProgram>& a_debugSceneShaderProgramResourceManager
		)
			: m_shaderProgram{ a_database }
			, m_debugSceneShaderProgramResourceManager{ a_debugSceneShaderProgramResourceManager  }
		{
			m_shaderProgram.setId(6);
			ignorable_assert(m_shaderProgram.isValid());
		}
	};
}
