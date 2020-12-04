#pragma once

#include <vob/aoe/core/type/ADynamicType.h>

#include <vob/aoe/common/render/gui/GuiRenderContext.h>
#include <vob/aoe/common/render/gui/GuiTransform.h>
#include <vob/aoe/common/render/gui/GuiShaderProgram.h>

namespace vob::aoe::common
{
	class AElement
		: public type::ADynamicType
	{
	public:
		// Methods
		virtual void render(
			GuiShaderProgram const& a_shaderProgram
			, GuiRenderContext& a_renderContext
			, GuiTransform a_transform
		) const = 0;
	};
}