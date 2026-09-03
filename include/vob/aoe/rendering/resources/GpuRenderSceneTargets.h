#pragma once

#include "vob/aoe/rendering/resources/GlResource.h"
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
		GlBuffer globalParamsUbo;
		GlBuffer viewParamsUbo;
		GlBuffer targetParamsUbo;
		GlBuffer lightViewParamsUbo;
		GlBuffer lightingParamsUbo;
		GlBuffer shadowParamsUbo;
		GlBuffer ssaoParamsUbo;
		GlBuffer ssrParamsUbo;
		GlBuffer bloomParamsUbo;
		GlBuffer tonemapParamsUbo;
		GlBuffer hudParamsUbo;
		GlBuffer debugParamsUbo;

		GlBuffer lightsSsbo;
		GlBuffer lightClusterSizesSsbo;
		GlBuffer lightClusterIndicesSsbo;
		GlBuffer skyIrradianceSsbo;

		GlTexture sunShadowMapDepthTextureArray;
		GpuRenderTarget sunShadowMap;

		struct SpotLightShadowMapTarget
		{
			GpuRenderTarget target;
			GlTexture depthTexture;
		};
		mistd::bounded_vector<SpotLightShadowMapTarget, k_spotLightShadowMapsCapacity>
			spotLightShadowMapTargets;

		glm::ivec2 shadingResolution;
		GlTexture directOpaqueColorTexture;
		GlTexture opaqueNormalTexture;
		GlTexture opaqueSurfaceTexture;
		GlTexture opaqueGeometricNormalTexture;
		GlTexture opaqueDepthTexture;
		GpuRenderTarget depthTarget;
		GpuRenderTarget directOpaqueTarget;

		GlTexture ssaoLinearDepthTexture;
		GlTexture ssaoRawOcclusionTexture;
		GlTexture ambientOcclusionTexture;
		GpuRenderTarget ssaoDepthTarget;
		GpuRenderTarget ssaoRawTarget;
		GpuRenderTarget ssaoTarget;

		GlTexture ssrColorTexture;
		GlTexture ssrRawColorTexture;
		GlTexture hiZDepthTexture;
		GpuRenderTarget ssrRawTarget;
		mistd::bounded_vector<GpuRenderTarget, k_hiZMipsCapacity> hiZMipTargets;
		mistd::bounded_vector<GpuRenderTarget, k_ssrMipsCapacity> ssrMipTargets;

		GlTexture finalColorTexture;
		GpuRenderTarget finalTarget;

		struct BloomMip
		{
			GpuRenderTarget target;
			GlTexture colorTexture;
		};
		mistd::bounded_vector<BloomMip, k_bloomMipsCapacity> bloomMips;

		GlTexture bloomCombinedColorTexture;
		GpuRenderTarget bloomCombinedTarget;

		struct PostProcessTarget
		{
			GpuRenderTarget target;
			GlTexture colorTexture;
		};
		std::array<PostProcessTarget, 2> postProcessTargets;

		GlVertexArray postProcessVao;
		GlVertexArray debugGeometryVao;
		GlBuffer debugGeometryVbo;
		GlBuffer debugGeometryEbo;

		GlProgram lightClusteringProgram;
		GlProgram ssaoDepthProgram;
		GlProgram ssaoProgram;
		GlProgram ssaoBlurProgram;
		GlProgram ssrProgram;
		GlProgram hiZReduceProgram;
		GlProgram ssrPrefilterProgram;
		GlProgram ssrDownsampleProgram;
		GlProgram opaqueCompositionProgram;
		GlProgram skyBoxProgram;
		GlProgram skyIrradianceProgram;
		GlProgram bloomDownsampleProgram;
		GlProgram bloomUpsampleProgram;
		GlProgram bloomCombineProgram;
		GlProgram tonemapProgram;
		GlProgram aaProgram;
		GlProgram presentProgram;
		GlProgram hudProgram;
		GlProgram debugProgram;
		GlProgram debugGeometryProgram;
	};
}
