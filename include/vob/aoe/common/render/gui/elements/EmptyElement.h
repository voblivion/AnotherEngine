#pragma once

#include <vob/aoe/common/render/gui/elements/StandardElement.h>


namespace vob::aoe::common
{
	class EmptyElement
		: public StandardElement
	{
	protected:
		void renderContent(GuiShaderProgram const& a_shaderProgram, GuiRenderContext& a_renderContext, GuiTransform a_transform) const override {}
	};
}
