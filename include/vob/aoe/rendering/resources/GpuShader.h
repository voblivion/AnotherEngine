#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"
#include "vob/aoe/rendering/MaterialParamsLayout.h"
#include "vob/aoe/rendering/ResolvedShader.h"
#include "vob/aoe/rendering/resources/GpuProgram.h"
#include "vob/aoe/rendering/resources/Handle.h"
#include "vob/aoe/rendering/ShadingPass.h"


namespace vob::aoegl
{
	struct GpuShader
	{
		struct OwnedProgram
		{
			Handle<GpuProgram> handle;
			GraphicId id = k_invalidId;
		};

		ShadingPass shadingPass;
		OwnedProgram staticProgram;
		OwnedProgram riggedProgram;
		OwnedProgram instancedProgram;
		OwnedProgram staticDepthProgram;
		OwnedProgram riggedDepthProgram;
		OwnedProgram instancedDepthProgram;
		OwnedProgram staticShadowMapProgram;
		OwnedProgram riggedShadowMapProgram;
		OwnedProgram instancedShadowMapProgram;
		bool isAlphaMasked;
		MaterialParamsLayout paramsLayout;

		ResolvedShader getResolvedStaticShader() const
		{
			return { staticProgram.id, staticDepthProgram.id, staticShadowMapProgram.id, isAlphaMasked };
		}

		ResolvedShader getResolvedRiggedShader() const
		{
			return { riggedProgram.id, riggedDepthProgram.id, riggedShadowMapProgram.id, isAlphaMasked };
		}

		ResolvedShader getResolvedInstancedShader() const
		{
			return {
				instancedProgram.id
				, instancedDepthProgram.id
				, instancedShadowMapProgram.id
				, isAlphaMasked
			};
		}
	};
}
