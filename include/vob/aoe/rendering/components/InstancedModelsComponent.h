#pragma once

#include <vob/aoe/rendering/GraphicTypes.h>
#include <vob/aoe/rendering/ShadingPass.h>

#include "vob/aoe/rendering/shaders/defines.h"

#include <cstdint>
#include <vector>


namespace vob::aoegl
{
	struct InstancedModelsComponent
	{
		struct Model
		{
			struct Mesh
			{
				ShadingPass shadingPass = ShadingPass::Opaque;
				GraphicId program = k_invalidId;
				int32_t materialIndex = -1;
				GraphicId meshVao = k_invalidId;
				int32_t indexCount = 0;
			};

			std::vector<Mesh> meshes;
			GraphicId instanceTransformVbo;
			int32_t instanceCount = 0;
		};

		std::vector<Model> models;
	};
}
