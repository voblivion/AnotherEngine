#pragma once

#include <vob/aoe/rendering/GraphicTypes.h>

#include <glm/glm.hpp>


namespace vob::aoegl
{
	struct alignas(16) GlobalMeshRenderingData
	{
		glm::mat4 viewProjection;
		glm::vec3 cameraPosition; float _cameraPositionPadding;
		glm::ivec2 clusterSizes;
		glm::ivec2 resolution;
		float near;
		float far;
		int32_t clusterCountZ;
		int32_t maxClusterLightCount;
	};

	struct alignas(16) GlobalLightClusteringData
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::ivec2 clusterSizes;
		glm::vec2 resolution;
		float near;
		float far;
		int32_t clusterCountZ;
		int32_t maxClusterLightCount;
		int32_t lightCount;
	};

	struct MeshRenderingContext
	{
		int32_t clusterCount = 0;
		int32_t maxLightCount = 2048;
		GraphicId lightClusteringProgram = k_invalidId;
		GraphicId globalClusteringDataUbo = k_invalidId;
		GraphicId lightsSsbo = k_invalidId;
		GraphicId clustersLightCountSsbo = k_invalidId;
		GraphicId clustersLightIndicesSsbo = k_invalidId;

	};
}
