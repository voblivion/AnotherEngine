#pragma once

#include <vob/aoe/common/_render/GraphicResourceHandle.h>
#include <vob/aoe/common/_render/gui/GuiMesh.h>
#include <vob/aoe/common/_render/Manager.h>

namespace vob::aoe::common
{
	struct GuiRenderContext
	{
		explicit GuiRenderContext(IGraphicResourceManager<GuiMesh>& a_guiMeshResourceManager)
			: m_quad{ a_guiMeshResourceManager }
		{}

		GraphicResourceHandle<GuiMesh> m_quad;
		// TimePoint m_frameStartTime;
	};
}