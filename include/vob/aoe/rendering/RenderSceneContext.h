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
		PostProcesses
	};

	struct NewRenderSceneContext
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
		GraphicId ambientOcclusionTexture;;

		struct SpotLightShadowMapTarget
		{
			glm::ivec2 resolution;
			GraphicId framebuffer;
			GraphicId depthTexture;
		};
		// TODO: sun's cascading shadow map
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
		struct PostProcess
		{
			GraphicId program;
			GraphicId ubo;

		};
		std::vector<PostProcess> postProcesses;
		GraphicId debugProgram;

		// Other
		GraphicId postProcessVao;
		std::array<GraphicId, 2> totalTimerQueries;
		mistd::enum_map<RenderPass, GraphicId> renderPassTimerQueries;
		GraphicId environmentCubeMap;

	};

	struct RenderSceneContext
	{
		struct PostProcess
		{
			GraphicId program;
			GraphicId ubo;
		};
		std::vector<PostProcess> postProcesses;

		GraphicId ssrUbo;
		GraphicId ssrProgram;
		GraphicId reflectionProgram;

		// Temporary : could ping pong
		GraphicId ssrFramebuffer;
		GraphicId ssrColorTexture;
		GraphicId reflectionFramebuffer;
		GraphicId reflectionColorTexture;

		mistd::enum_map<RenderPass, GraphicId> renderPassTimerQueries;
		
		std::array<GraphicId, 10> timerQueries;
		glm::ivec2 sceneFramebufferSize;
		int32_t maxLightCount;
		// TODO: x, y, z where?
		int32_t lightClusterCount;

		GraphicId sceneFramebuffer;
		GraphicId sceneColorTexture;
		GraphicId sceneNormalTexture;
		GraphicId sceneSurfaceTexture;
		GraphicId sceneDepthTexture;
		GraphicId postProcessFramebuffer;
		GraphicId postProcessColorTexture;
		// MSAA
		//GraphicId resolveFramebuffer;
		//GraphicId resolveColorTexture;
		// FXAA
		GraphicId postProcessVao;

		// Shadow
		struct ShadowMap
		{
			GraphicId framebuffer;
			GraphicId depthTexture;
			glm::ivec2 size;
		};

		glm::vec3 sunDirection = glm::normalize(glm::vec3{ 0.3f, -0.6f, 0.7f });
		GraphicId lightViewParamsUbo = k_invalidId;
		ShadowMap sunShadowMap;
		mistd::bounded_vector<ShadowMap, k_maxSpotLightShadowMapCount> spotLightShadowMaps;

		// Common
		GraphicId globalParamsUbo = k_invalidId;
		GraphicId viewParamsUbo = k_invalidId;
		GraphicId lightingParamsUbo = k_invalidId;
		GraphicId shadowParamsUbo = k_invalidId;
		GraphicId lightsSsbo = k_invalidId;
		GraphicId lightClusterSizesSsbo = k_invalidId;
		GraphicId lightClusterIndicesSsbo = k_invalidId;

		// Debug
		GraphicId debugPostProcessProgram = k_invalidId;
		GraphicId debugParamsUbo = k_invalidId;

		// Light Clustering
		int32_t lightClusteringWorkGroupSize = 0;
		GraphicId lightClusteringProgram = k_invalidId;

		// Depth
		GraphicId staticDepthProgram = k_invalidId;
		GraphicId riggedDepthProgram = k_invalidId;
	};
}
