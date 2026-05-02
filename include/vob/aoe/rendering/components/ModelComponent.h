#pragma once

#include <vob/aoe/rendering/GraphicTypes.h>
#include <vob/aoe/rendering/ShadingPass.h>

#include "vob/aoe/rendering/shaders/defines.h"

#include <cstdint>
#include <vector>


namespace vob::aoegl
{
	struct ModelComponentMesh
	{
		ShadingPass shadingPass = ShadingPass::Opaque;
		GraphicId program = k_invalidId;
		int32_t materialIndex = k_invalidId;
		GraphicId meshVao = k_invalidId;
		int32_t indexCount = 0;
	};

	struct StaticModelComponent
	{
		std::pmr::vector<ModelComponentMesh> meshes;
	};

	struct RiggedModelComponent
	{
		std::pmr::vector<ModelComponentMesh> meshes;
		GraphicId rigParamsUbo = k_invalidId;
	};
}
