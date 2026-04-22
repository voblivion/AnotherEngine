#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"

#include <vob/misc/std/enum_traits.h>
#include <vob/misc/std/bounded_vector.h>
#include <vob/misc/std/enum_map.h>

#include <chrono>


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

	enum class RenderPass : uint8_t
	{
		LightClustering = 0,
		ShadowMaps,
		DepthPrePass,
		DirectOpaque,
		SSAO,
		SSR,
		OpaqueComposition,
		Translucent,
		SkyBox,
		PostProcesses
	};

	struct RenderSceneContext
	{

		// Parameters
		int32_t lightsCapacity = 2048;
		glm::ivec2 lightClusterTileSize = glm::ivec2{ 16 };
		int32_t lightClusterZCount = 24;
		int32_t lightClusterCapacity = 64;
		int32_t lightClusteringWorkGroupSize = 128;

		// Uniform Buffer Objects
		GraphicId globalParamsUbo;
		GraphicId viewParamsUbo;
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

		// Framebuffers
		GraphicId depthFramebuffer;

		GraphicId ssaoFramebuffer;
		GraphicId ambientOcclusionTexture;

		glm::vec3 sunDir = glm::normalize(glm::vec3{ 0.0f, 0.15f, -1.0f });
		glm::ivec2 sunShadowMapResolution;
		GraphicId sunShadowMapFramebuffer;
		GraphicId sunShadowMapDepthTextureArray;
		mistd::bounded_vector<float, k_sunCascadingShadowMapsCapacity> sunShadowMapFrustumFarClips = { 10.0f, 40.0f, 200.0f, 1000.0f };
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
		GraphicId opaqueSurfaceTexture; // r = reflectance, g = roughness (for now)
		GraphicId opaqueGeometricNormalTexture;
		GraphicId opaqueDepthTexture;

		// TODO: don't like name `lit`

		glm::ivec2 ssrResolution;
		int32_t ssrMipLevels;
		GraphicId ssrFramebuffer;
		GraphicId ssrColorTexture;

		GraphicId finalFramebuffer;
		GraphicId finalColorTexture;

		glm::ivec2 postProcessResolution;
		struct PostProcessTarget
		{
			GraphicId framebuffer;
			GraphicId colorTexture;
		};
		std::array<PostProcessTarget, 2> postProcessTargets;

		// Programs
		GraphicId lightClusteringProgram; // out: none
		GraphicId staticShadowMapProgram;
		GraphicId riggedShadowMapProgram;
		GraphicId staticDepthProgram; // out: directOpaqueFramebuffer
		GraphicId riggedDepthProgram; // out: directOpaqueFramebuffer
		GraphicId ssaoProgram; // out: litOpaqueFramebuffer
		GraphicId ssrProgram; // out: ssrFramebuffer
		GraphicId opaqueCompositionProgram; // out: finalFramebuffer
		GraphicId skyBoxProgram = k_invalidId;
		struct PostProcess
		{
			GraphicId program;
			GraphicId ubo;

		};
		std::vector<PostProcess> postProcesses;
		GraphicId debugProgram;
		GraphicId debugGeometryProgram;

		// Other
		GraphicId debugGeometryVao;
		GraphicId debugGeometryVbo;
		GraphicId debugGeometryEbo;
		GraphicId postProcessVao;
		std::array<GraphicId, 2> totalTimerQueries;
		mistd::enum_map<RenderPass, GraphicId> renderPassTimerQueries;
		GraphicId environmentCubeMap;

	};
}
