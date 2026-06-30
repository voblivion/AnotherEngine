#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"


namespace vob::aoegl
{
	struct GpuMesh
	{
		GraphicId vao;
		GraphicId vbo;
		GraphicId ebo;
		int32_t indexCount;
	};
}
