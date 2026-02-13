#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"

#include <vob/misc/std/enum_traits.h>


#define VOB_AOEGL_DEBUG 1

namespace vob::aoegl
{
	struct DebugMode
	{
		enum Type : int32_t
		{
			None,
			Depths,
			LightClusters,
			Normals,
			LocalNormals,
			UVs,
			Count
		};
	};
}

namespace vob::mistd
{
	template <>
	struct reflected_enum_range<vob::aoegl::DebugMode::Type>
	{
		static constexpr auto begin = vob::aoegl::DebugMode::Type::None;
		static constexpr auto end = vob::aoegl::DebugMode::Type::Count;
	};
}

namespace vob::aoegl
{
	struct DebugPostProcessConfig
	{
		DebugMode::Type mode;
	};

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

	struct alignas(16) CulledLight2
	{
		glm::vec4 positionAndRadius;
		glm::vec4 colorAndIntensity;
		glm::vec4 directionAndType;
		glm::vec4 spotCosAngles;
	};

	struct RenderSceneContext
	{
		GraphicId globalUbo = k_invalidId;
		GraphicId meshUbo = k_invalidId;
		GraphicId postProcessUbo = k_invalidId;

		GraphicId lightsSsbo = k_invalidId;
		GraphicId lightCountPerClusterSsbo = k_invalidId;
		GraphicId lightIndicesPerClusterSsbo = k_invalidId;

		GraphicId computeLightClustersProgram = k_invalidId;
		GraphicId depthPrePassProgram = k_invalidId;
		GraphicId postProcessProgram = k_invalidId;

#ifdef VOB_AOEGL_DEBUG
		GraphicId debugMeshProgram = k_invalidId;
		GraphicId debugPostProcessProgram = k_invalidId;
		GraphicId debugUbo = k_invalidId;

		std::array<GraphicId, 10> timerQueries;
#endif


		glm::ivec2 sceneFramebufferSize;
		int32_t maxLightCount;
		// TODO: x, y, z where?
		int32_t lightClusterCount;
		GraphicId sceneFramebuffer;
		GraphicId sceneColorTexture;
		GraphicId sceneDepthTexture;
		GraphicId postProcessVao;
	};
}
