#pragma once

#include <vob/aoe/core/type/ADynamicType.h>

#include <vob/aoe/common/render/gui/GuiRenderContext.h>
#include <vob/aoe/common/render/gui/GuiTransform.h>
#include <vob/aoe/common/render/gui/GuiShaderProgram.h>
#include <vob/aoe/common/render/IWindow.h>

namespace vob::aoe::common
{
	class VOB_AOE_API AElement
		: public type::ADynamicType
	{
	public:
		// Methods
		virtual void render(
			GuiShaderProgram const& a_shaderProgram
			, GuiRenderContext& a_renderContext
			, GuiTransform a_transform
		) const = 0;

		virtual bool onEvent(WindowEvent const& a_event, GuiTransform a_transform)
		{
			return false;
		}
	};
}