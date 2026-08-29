#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"
#include "vob/aoe/rendering/RenderSceneConfig.h"
#include "vob/aoe/rendering/shaders/defines.h"

#include <vob/misc/std/enum_traits.h>
#include <vob/misc/std/bounded_vector.h>
#include <vob/misc/std/enum_map.h>

#include <glm/glm.hpp>

#include <chrono>
#include <functional>


namespace vob::aoegl
{
	struct alignas(16) GlobalRenderSceneConfig
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 viewProjection;
		glm::mat4 invProjection;
		glm::vec3 cameraPosition; float _cameraPositionPad;
		glm::ivec2 resolution;
		float nearClip;
		float farClip;

		glm::ivec2 lightClusterSizes;
		int32_t lightClusterCountZ;
		int32_t maxLightCountPerCluster;
		int32_t totalLightCount;
	};

	struct alignas(16) MeshRenderSceneConfig
	{
		glm::mat4 model;
	};

	constexpr int32_t k_bloomMipsCapacity = 12;
	constexpr int32_t k_ssrMipsCapacity = 16;
	constexpr int32_t k_hiZMipsCapacity = 16;

	struct RenderSceneContext
	{
		std::reference_wrapper<RenderSceneConfig> config;

		// Parameters
		int32_t lightClusteringWorkGroupSize = 128;

		// Uniform Buffer Objects
		GraphicId globalParamsUbo;
		GraphicId viewParamsUbo;
		GraphicId targetParamsUbo;
		GraphicId lightViewParamsUbo;
		GraphicId lightingParamsUbo;
		GraphicId shadowParamsUbo;
		GraphicId ssaoParamsUbo;
		GraphicId ssrParamsUbo = k_invalidId;
		GraphicId debugParamsUbo;

		// Shader Storage Buffer Objects
		GraphicId lightsSsbo;
		GraphicId lightClusterSizesSsbo;
		GraphicId lightClusterIndicesSsbo;
		GraphicId skyIrradianceSsbo;
		GraphicId skyIrradianceProgram;

		// Framebuffers
		GraphicId depthFramebuffer;

		glm::ivec2 ssaoResolution;
		GraphicId ssaoDepthFramebuffer;
		GraphicId ssaoLinearDepthTexture;
		GraphicId ssaoRawFramebuffer;
		GraphicId ssaoRawOcclusionTexture;
		GraphicId ssaoFramebuffer;
		GraphicId ambientOcclusionTexture;

		glm::vec3 sunDir = glm::normalize(glm::vec3{ 0.7f, 0.15f, -1.0f });
		GraphicId sunShadowMapFramebuffer;
		GraphicId sunShadowMapDepthTextureArray;
		struct SpotLightShadowMapTarget
		{
			glm::ivec2 resolution;
			GraphicId framebuffer;
			GraphicId depthTexture;
		};
		std::array<SpotLightShadowMapTarget, k_spotLightShadowMapsCapacity> spotLightShadowMapTargets;

		glm::ivec2 shadingResolution;
		GraphicId directOpaqueFramebuffer;
		GraphicId directOpaqueColorTexture;
		GraphicId opaqueNormalTexture;
		GraphicId opaqueSurfaceTexture; // rgb = f0, a = roughness
		GraphicId opaqueGeometricNormalTexture;
		GraphicId opaqueDepthTexture;

		glm::ivec2 ssrResolution;
		int32_t ssrMipLevels;
		GraphicId ssrColorTexture;
		GraphicId ssrRawColorTexture;
		GraphicId ssrRawFramebuffer;
		int32_t hiZMipLevels;
		GraphicId hiZDepthTexture;
		mistd::bounded_vector<GraphicId, k_hiZMipsCapacity> hiZMipFramebuffers;
		mistd::bounded_vector<GraphicId, k_ssrMipsCapacity> ssrMipFramebuffers;

		GraphicId finalFramebuffer;
		GraphicId finalColorTexture;

		struct BloomMip
		{
			GraphicId framebuffer;
			GraphicId colorTexture;
			glm::ivec2 resolution;
		};
		mistd::bounded_vector<BloomMip, k_bloomMipsCapacity> bloomMips;

		GraphicId bloomCombinedFramebuffer;
		GraphicId bloomCombinedColorTexture;

		struct PostProcessTarget
		{
			GraphicId framebuffer;
			GraphicId colorTexture;
		};
		std::array<PostProcessTarget, 2> postProcessTargets;

		// Programs
		GraphicId lightClusteringProgram;
		GraphicId ssaoDepthProgram;
		GraphicId ssaoProgram;
		GraphicId ssaoBlurProgram;
		GraphicId ssrProgram;
		GraphicId hiZReduceProgram;
		GraphicId ssrPrefilterProgram;
		GraphicId ssrDownsampleProgram;
		GraphicId opaqueCompositionProgram;
		GraphicId skyBoxProgram = k_invalidId;
		GraphicId bloomDownsampleProgram;
		GraphicId bloomUpsampleProgram;
		GraphicId bloomCombineProgram;
		GraphicId bloomParamsUbo;
		GraphicId tonemapProgram;
		GraphicId tonemapParamsUbo;
		GraphicId aaProgram;
		GraphicId presentProgram;
		GraphicId hudProgram;
		GraphicId hudParamsUbo;
		GraphicId debugProgram;
		GraphicId debugGeometryProgram;

		// Other
		GraphicId debugGeometryVao;
		GraphicId debugGeometryVbo;
		GraphicId debugGeometryEbo;
		GraphicId postProcessVao;

	};
}
