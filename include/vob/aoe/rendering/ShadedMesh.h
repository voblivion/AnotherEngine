#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"
#include "vob/aoe/rendering/ResolvedShader.h"
#include "vob/aoe/rendering/ShadingPass.h"
#include "vob/aoe/rendering/resources/WeakHandle.h"


namespace vob::aoegl
{
	struct GpuMaterial;
	struct GpuProgram;

	struct ShadedMesh
	{
		ShadingPass shadingPass = ShadingPass::Opaque;
		ResolvedShader shader;
		WeakHandle<GpuMaterial> material;
		GraphicId vao;
		int32_t indexCount = 0;
	};
}
