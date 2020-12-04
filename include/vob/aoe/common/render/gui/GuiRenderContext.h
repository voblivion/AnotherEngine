#pragma once

#include <vob/aoe/common/render/gui/GuiMesh.h>

namespace vob::aoe::common
{
	struct GuiRenderContext
	{
		explicit GuiRenderContext(IGraphicResourceManager<GuiMesh>& a_guiMeshResourceManager)
			: m_quad{ a_guiMeshResourceManager }
		{}

		GraphicResourceHandle<GuiMesh> m_quad;
	};
}