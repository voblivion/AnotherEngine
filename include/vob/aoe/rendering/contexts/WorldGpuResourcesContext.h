#pragma once

#include "vob/aoe/rendering/resources/GpuMaterial.h"
#include "vob/aoe/rendering/resources/GpuMesh.h"
#include "vob/aoe/rendering/resources/GpuShader.h"
#include "vob/aoe/rendering/resources/Handle.h"


namespace vob::aoegl
{
	// TODO: probably a temporary context until I have a better way to reference "world gpu resources to keep loaded".
	struct WorldGpuResourcesContext
	{
		std::vector<Handle<GpuMaterial>> materials;
		std::vector<Handle<GpuMesh>> meshes;
	};
}
