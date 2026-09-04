#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"
#include "vob/aoe/rendering/ResolvedShader.h"
#include "vob/aoe/rendering/ShadingPass.h"
#include "vob/aoe/rendering/resources/GpuMesh.h"

#include <memory>


namespace vob::aoegl
{
	struct GpuMaterial;

	struct ShadedMesh
	{
		ShadingPass shadingPass = ShadingPass::Opaque;
		ResolvedShader shader;
		std::shared_ptr<GpuMaterial> material;
		std::shared_ptr<GpuMesh> mesh;
	};
}
