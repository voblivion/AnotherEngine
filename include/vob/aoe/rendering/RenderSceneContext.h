#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"

#include <vob/misc/std/enum_traits.h>


#define VOB_AOEGL_DEBUG 1

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

	struct RenderSceneContext
	{
		GraphicId postProcessUbo = k_invalidId;
		GraphicId postProcessProgram = k_invalidId;
		std::array<GraphicId, 10> timerQueries;
		glm::ivec2 sceneFramebufferSize;
		int32_t maxLightCount;
		// TODO: x, y, z where?
		int32_t lightClusterCount;
		GraphicId sceneFramebuffer;
		GraphicId sceneColorTexture;
		GraphicId sceneDepthTexture;
		GraphicId postProcessVao;

		// Common
		GraphicId viewParamsUbo = k_invalidId;
		GraphicId lightParamsUbo = k_invalidId;
		GraphicId lightsSsbo = k_invalidId;
		GraphicId lightClusterSizesSsbo = k_invalidId;
		GraphicId lightClusterIndicesSsbo = k_invalidId;

		// Debug
		GraphicId staticDebugForwardProgram = k_invalidId;
		GraphicId riggedDebugForwardProgram = k_invalidId;
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
