#pragma once

#include <vob/misc/std/ignorable_assert.h>

#include <vob/aoe/ecs/Component.h>
#include <vob/aoe/core/data/ADatabase.h>

#include <vob/aoe/common/render/gui/GuiShaderProgram.h>
#include <vob/aoe/common/render/GraphicResourceHandle.h>
#include <vob/aoe/common/render/gui/GuiRenderContext.h>
#include <vob/aoe/common/render/gui/GuiMesh.h>

namespace vob::aoe::common
{
	struct GuiRenderComponent final
		: public aoecs::AComponent
	{
		// Attributes
		std::shared_ptr<GraphicResourceHandle<GuiShaderProgram> const> m_shaderProgram;
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
			: m_shaderProgramResourceManager{ a_shaderProgramResourceManager }
			, m_guiMeshResourceManager{ a_guiMeshResourceManager }
			, m_textureResourceManager{ a_textureResourceManager }
			, m_guiRenderContext{ a_guiMeshResourceManager }
		{
			m_shaderProgram = a_database.find<GraphicResourceHandle<GuiShaderProgram>>(8);
			ignorable_assert(m_shaderProgram != nullptr);
		}
	};
}
