#pragma once

#include <vob/aoe/rendering/GraphicTypes.h>
#include <vob/aoe/rendering/ShadedMesh.h>
#include <vob/aoe/rendering/resources/GpuBuffer.h>
#include <vob/aoe/rendering/resources/Handle.h>

#include "vob/aoe/rendering/shaders/defines.h"

#include <cstdint>
#include <vector>


namespace vob::aoegl
{
	struct InstancedModelsComponent
	{
		struct Model
		{
			std::vector<ShadedMesh> meshes;
			GraphicId instanceTransformsVbo;
			Handle<GpuBuffer> instanceTransforms;
			int32_t instanceCount = 0;
		};

		std::vector<Model> models;
	};
}
