#pragma once

#include <vob/aoe/rendering/GraphicTypes.h>

#include <cstdint>
#include <vector>


namespace vob::aoegl
{
	struct ModelComponentMesh
	{
		GraphicId program = k_invalidId;
		GraphicId materialIndex = k_invalidId;
		GraphicId meshVao = k_invalidId;
		int32_t indexCount = 0;
	};

	struct StaticModelComponent
	{
		std::pmr::vector<ModelComponentMesh> meshes;
		GraphicId modelParamsUbo = k_invalidId;
		float boundingRadius = 0.0f;
	};

	struct RiggedModelComponent
	{
		std::pmr::vector<ModelComponentMesh> meshes;
		GraphicId modelParamsUbo = k_invalidId;
		GraphicId rigParamsUbo = k_invalidId;
		float boundingRadius = 0.0f;
	};
}
