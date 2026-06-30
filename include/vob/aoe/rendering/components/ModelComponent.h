#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"
#include "vob/aoe/rendering/ShadedMesh.h"

#include "vob/aoe/rendering/shaders/defines.h"

#include <cstdint>
#include <vector>


namespace vob::aoegl
{
	struct StaticModelComponent
	{
		std::pmr::vector<ShadedMesh> meshes;
	};

	struct RiggedModelComponent
	{
		std::pmr::vector<ShadedMesh> meshes;
		GraphicId rigParamsUbo = k_invalidId;
	};
}
