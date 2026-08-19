#pragma once

#include "vob/aoe/rendering/resources/GpuMaterial.h"
#include "vob/aoe/rendering/resources/Handle.h"

#include <string>
#include <vector>


namespace vob::aoegl
{
	struct DebugMaterialContext
	{
		struct Material
		{
			std::string shaderName;
			std::string name;
			Handle<GpuMaterial> material;
		};

		std::vector<Material> materials;
		int32_t activeMaterialIndex = 0;
	};
}
