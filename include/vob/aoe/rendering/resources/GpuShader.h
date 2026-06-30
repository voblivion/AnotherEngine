#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"
#include "vob/aoe/rendering/ShadingPass.h"


namespace vob::aoegl
{
	struct GpuShader
	{
		ShadingPass shadingPass;
		GraphicId staticProgram;
		GraphicId riggedProgram;
		GraphicId instancedProgram;
	};
}
