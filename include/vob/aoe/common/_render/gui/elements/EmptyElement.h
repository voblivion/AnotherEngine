#pragma once

#include <vob/aoe/common/_render/gui/elements/AStandardElement.h>


namespace vob::aoe::common
{
	class EmptyElement
		: public AStandardElement
	{
	protected:
		void renderContent(
			GuiShaderProgram const& a_shaderProgram
			, GuiRenderContext& a_renderContext
			, GuiTransform a_transform
		) const override {}
	};
}
