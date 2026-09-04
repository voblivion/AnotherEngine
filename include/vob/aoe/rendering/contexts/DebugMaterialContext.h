#pragma once

#include "vob/aoe/rendering/resources/GpuMaterial.h"

#include <memory>
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
			std::weak_ptr<GpuMaterial> material;
		};

		std::vector<Material> materials;
		int32_t activeMaterialIndex = 0;
	};
}
