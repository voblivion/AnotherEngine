#pragma once

#include "vob/aoe/rendering/resources/GpuMesh.h"

#include <memory>
#include <vector>


namespace vob::aoegl
{
	struct Model
	{
		std::vector<std::shared_ptr<GpuMesh>> meshes;
		float boundingRadius = 0.0f;
	};
}
