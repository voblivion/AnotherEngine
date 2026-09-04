#pragma once

#include "vob/aoe/rendering/resources/GpuResource.h"
#include "vob/aoe/rendering/resources/GpuRenderTarget.h"
#include "vob/aoe/rendering/shaders/defines.h"

#include <vob/misc/std/bounded_vector.h>

#include <glm/glm.hpp>

#include <array>


namespace vob::aoegl
{
	constexpr int32_t k_bloomMipsCapacity = 12;
	constexpr int32_t k_ssrMipsCapacity = 16;
	constexpr int32_t k_hiZMipsCapacity = 16;

	struct GpuRenderSceneTargets
	{
		GpuBuffer globalParamsUbo;
		GpuBuffer viewParamsUbo;
		GpuBuffer targetParamsUbo;
		GpuBuffer lightViewParamsUbo;
		GpuBuffer lightingParamsUbo;
		GpuBuffer shadowParamsUbo;
		GpuBuffer ssaoParamsUbo;
		GpuBuffer ssrParamsUbo;
		GpuBuffer bloomParamsUbo;
		GpuBuffer tonemapParamsUbo;
		GpuBuffer hudParamsUbo;
		GpuBuffer debugParamsUbo;

		GpuBuffer lightsSsbo;
		GpuBuffer lightClusterSizesSsbo;
		GpuBuffer lightClusterIndicesSsbo;
		GpuBuffer skyIrradianceSsbo;

		GpuTexture sunShadowMapDepthTextureArray;
		GpuRenderTarget sunShadowMap;

		struct SpotLightShadowMapTarget
		{
			GpuRenderTarget target;
			GpuTexture depthTexture;
		};
		mistd::bounded_vector<SpotLightShadowMapTarget, k_spotLightShadowMapsCapacity>
			spotLightShadowMapTargets;

		glm::ivec2 shadingResolution;
		GpuTexture directOpaqueColorTexture;
		GpuTexture opaqueNormalTexture;
		GpuTexture opaqueSurfaceTexture;
		GpuTexture opaqueGeometricNormalTexture;
		GpuTexture opaqueDepthTexture;
		GpuRenderTarget depthTarget;
		GpuRenderTarget directOpaqueTarget;

		GpuTexture ssaoLinearDepthTexture;
		GpuTexture ssaoRawOcclusionTexture;
		GpuTexture ambientOcclusionTexture;
		GpuRenderTarget ssaoDepthTarget;
		GpuRenderTarget ssaoRawTarget;
		GpuRenderTarget ssaoTarget;

		GpuTexture ssrColorTexture;
		GpuTexture ssrRawColorTexture;
		GpuTexture hiZDepthTexture;
		GpuRenderTarget ssrRawTarget;
		mistd::bounded_vector<GpuRenderTarget, k_hiZMipsCapacity> hiZMipTargets;
		mistd::bounded_vector<GpuRenderTarget, k_ssrMipsCapacity> ssrMipTargets;

		GpuTexture finalColorTexture;
		GpuRenderTarget finalTarget;

		struct BloomMip
		{
			GpuRenderTarget target;
			GpuTexture colorTexture;
		};
		mistd::bounded_vector<BloomMip, k_bloomMipsCapacity> bloomMips;

		GpuTexture bloomCombinedColorTexture;
		GpuRenderTarget bloomCombinedTarget;

		struct PostProcessTarget
		{
			GpuRenderTarget target;
			GpuTexture colorTexture;
		};
		std::array<PostProcessTarget, 2> postProcessTargets;

		GpuVertexArray postProcessVao;
		GpuVertexArray debugGeometryVao;
		GpuBuffer debugGeometryVbo;
		GpuBuffer debugGeometryEbo;

		GpuProgram lightClusteringProgram;
		GpuProgram ssaoDepthProgram;
		GpuProgram ssaoProgram;
		GpuProgram ssaoBlurProgram;
		GpuProgram ssrProgram;
		GpuProgram hiZReduceProgram;
		GpuProgram ssrPrefilterProgram;
		GpuProgram ssrDownsampleProgram;
		GpuProgram opaqueCompositionProgram;
		GpuProgram skyBoxProgram;
		GpuProgram skyIrradianceProgram;
		GpuProgram bloomDownsampleProgram;
		GpuProgram bloomUpsampleProgram;
		GpuProgram bloomCombineProgram;
		GpuProgram tonemapProgram;
		GpuProgram aaProgram;
		GpuProgram presentProgram;
		GpuProgram hudProgram;
		GpuProgram debugProgram;
		GpuProgram debugGeometryProgram;
	};
}
