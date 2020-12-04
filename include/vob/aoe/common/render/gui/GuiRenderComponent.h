#pragma once

#include <vob/sta/ignorable_assert.h>

#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/core/data/ADatabase.h>
#include <vob/aoe/core/data/Handle.h>

#include <vob/aoe/common/render/gui/GuiShaderProgram.h>
#include <vob/aoe/common/render/GraphicResourceHandle.h>
#include <vob/aoe/common/render/gui/GuiRenderContext.h>
#include <vob/aoe/common/render/gui/GuiMesh.h>

namespace vob::aoe::common
{
	struct GuiRenderComponent final
		: public ecs::AComponent
	{
		// Attributes
		data::Handle<GraphicResourceHandle<GuiShaderProgram>> m_shaderProgram;
		IGraphicResourceManager<GuiShaderProgram>& m_shaderProgramResourceManager;
		IGraphicResourceManager<GuiMesh>& m_guiMeshResourceManager;
		IGraphicResourceManager<Texture>& m_textureResourceManager;
		GuiRenderContext m_guiRenderContext;

		explicit GuiRenderComponent(
			data::ADatabase& a_database
			, IGraphicResourceManager<GuiShaderProgram>& a_shaderProgramResourceManager
			, IGraphicResourceManager<GuiMesh>& a_guiMeshResourceManager
			, IGraphicResourceManager<Texture>& a_textureResourceManager
		)
			: m_shaderProgram{ a_database }
			, m_shaderProgramResourceManager{ a_shaderProgramResourceManager }
			, m_guiMeshResourceManager{ a_guiMeshResourceManager }
			, m_textureResourceManager{ a_textureResourceManager }
			, m_guiRenderContext{ a_guiMeshResourceManager }
			, m_textElementTest{ a_database, m_guiMeshResourceManager }
		{
			m_shaderProgram.setId(8);
			ignorable_assert(m_shaderProgram.isValid());
		}

		TextElement m_textElementTest;
	};
}
