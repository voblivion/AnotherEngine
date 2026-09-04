#pragma once

#include "vob/aoe/rendering/resources/GpuResource.h"


namespace vob::aoegl
{
	struct GpuMesh
	{
		GpuVertexArray vao;
		GpuBuffer vbo;
		GpuBuffer ebo;
		int32_t indexCount;
	};
}
