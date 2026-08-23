#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"
#include "vob/aoe/rendering/MaterialParamsLayout.h"
#include "vob/aoe/rendering/ShadingPass.h"


namespace vob::aoegl
{
	struct GpuShader
	{
		ShadingPass shadingPass;
		GraphicId staticProgram;
		GraphicId riggedProgram;
		GraphicId instancedProgram;
		// TODO: shaders that need no variant of their own alias the core programs, so these are NOT owned
		// and must not be deleted with the shader. Promote to a shared GpuProgram resource once shader
		// definitions can be created and destroyed at runtime.
		GraphicId staticDepthProgram;
		GraphicId riggedDepthProgram;
		GraphicId instancedDepthProgram;
		GraphicId staticShadowMapProgram;
		GraphicId riggedShadowMapProgram;
		GraphicId instancedShadowMapProgram;
		MaterialParamsLayout paramsLayout;
	};
}
