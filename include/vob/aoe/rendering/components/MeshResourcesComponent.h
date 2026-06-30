#pragma once

#include "vob/aoe/rendering/resources/GpuMesh.h"
#include "vob/aoe/rendering/resources/Handle.h"

#include <vector>


namespace vob::aoegl
{
	struct MeshResourcesComponent
	{
		std::vector<Handle<GpuMesh>> meshes;
	};
}
