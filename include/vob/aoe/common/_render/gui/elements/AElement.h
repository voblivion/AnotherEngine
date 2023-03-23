#pragma once

#include <vob/aoe/common/_render/gui/GuiRenderContext.h>
#include <vob/aoe/common/_render/gui/GuiTransform.h>
#include <vob/aoe/common/_render/gui/GuiShaderProgram.h>
#include <vob/aoe/common/_render/IWindow.h>

#include <vob/aoe/core/type/ADynamicType.h>

namespace vob::aoe::common
{
	class VOB_AOE_API AElement
		: public type::ADynamicType
	{
	public:

		virtual ~AElement() = default;

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