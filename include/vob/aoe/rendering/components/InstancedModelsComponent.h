#pragma once

#include <vob/aoe/rendering/GraphicTypes.h>
#include <vob/aoe/rendering/ShadedMesh.h>
#include <vob/aoe/rendering/resources/GpuResource.h>

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
			GpuBuffer instanceTransformsVbo;
			int32_t instanceCount = 0;
		};

		InstancedModelsComponent() = default;
		InstancedModelsComponent(InstancedModelsComponent const&) = delete;
		InstancedModelsComponent& operator=(InstancedModelsComponent const&) = delete;
		InstancedModelsComponent(InstancedModelsComponent&&) = default;
		InstancedModelsComponent& operator=(InstancedModelsComponent&&) = default;

		std::vector<Model> models;
	};
}
