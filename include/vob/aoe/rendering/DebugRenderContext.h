#pragma once

#include <vob/aoe/rendering/GraphicTypes.h>
#include <vob/aoe/rendering/Program.h>


namespace vob::aoegl
{
	struct DebugRenderContext
	{
		GraphicId vao;
		GraphicId vbo;
		GraphicId ebo;
		DebugProgram debugProgram;
	};
}
