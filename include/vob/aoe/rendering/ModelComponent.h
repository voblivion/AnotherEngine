#pragma once

#include <vob/aoe/rendering/GraphicTypes.h>
#include <vob/aoe/rendering/GpuObjects.h>

#include "vob/aoe/rendering/shaders/defines.h"

#include <cstdint>
#include <vector>


namespace vob::aoegl
{
	enum class ShadingPass
	{
		Opaque,
		Translucent
	};

	struct ModelComponentMesh
	{
		ShadingPass shadingPass;
		GraphicId oldProgram = k_invalidId;
		GraphicId program = k_invalidId;
		GraphicId materialIndex = k_invalidId;
		GraphicId meshVao = k_invalidId;
		int32_t indexCount = 0;
	};

	struct StaticModelComponent
	{
		std::pmr::vector<ModelComponentMesh> meshes;
		// TODO: maybe move elsewhere
		ModelParams oldModelParams;
		// TODO: maybe move elsewhere
		UniformModelParams modelParams;
		GraphicId modelParamsUbo = k_invalidId;
		float boundingRadius = 0.0f;
	};

	struct RiggedModelComponent
	{
		std::pmr::vector<ModelComponentMesh> meshes;
		// TODO: maybe move elsewhere
		ModelParams oldModelParams;
		UniformModelParams modelParams;
		GraphicId modelParamsUbo = k_invalidId;
		GraphicId rigParamsUbo = k_invalidId;
		float boundingRadius = 0.0f;
	};
}
