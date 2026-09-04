#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"
#include "vob/aoe/rendering/MaterialParamsLayout.h"
#include "vob/aoe/rendering/ResolvedShader.h"
#include "vob/aoe/rendering/resources/GpuResource.h"
#include "vob/aoe/rendering/ShadingPass.h"

#include <memory>


namespace vob::aoegl
{
	struct GpuShader
	{
		ShadingPass shadingPass;
		GpuProgram staticProgram;
		GpuProgram riggedProgram;
		GpuProgram instancedProgram;
		std::shared_ptr<GpuProgram> staticDepthProgram;
		std::shared_ptr<GpuProgram> riggedDepthProgram;
		std::shared_ptr<GpuProgram> instancedDepthProgram;
		std::shared_ptr<GpuProgram> staticShadowMapProgram;
		std::shared_ptr<GpuProgram> riggedShadowMapProgram;
		std::shared_ptr<GpuProgram> instancedShadowMapProgram;
		bool isAlphaMasked;
		MaterialParamsLayout paramsLayout;

		ResolvedShader getResolvedStaticShader() const
		{
			return { staticProgram, *staticDepthProgram, *staticShadowMapProgram, isAlphaMasked };
		}

		ResolvedShader getResolvedRiggedShader() const
		{
			return { riggedProgram, *riggedDepthProgram, *riggedShadowMapProgram, isAlphaMasked };
		}

		ResolvedShader getResolvedInstancedShader() const
		{
			return {
				instancedProgram
				, *instancedDepthProgram
				, *instancedShadowMapProgram
				, isAlphaMasked
			};
		}
	};
}
