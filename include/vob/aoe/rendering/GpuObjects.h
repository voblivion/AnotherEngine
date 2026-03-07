#pragma once

#include <vob/aoe/rendering/GraphicTypes.h>

#include <vob/misc/std/enum_traits.h>

#include <glm/glm.hpp>

#include <cstdint>


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
			DebugOnly,
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
	constexpr static GraphicInt k_viewParamsUboLocation = 0;
	constexpr static GraphicInt k_lightParamsUboLocation = 1;
	constexpr static GraphicInt k_modelParamsUboLocation = 2;
	constexpr static GraphicInt k_rigParamsUboLocation = 3;
	constexpr static GraphicInt k_materialParamsUboLocation = 4;

	constexpr static GraphicInt k_lightsSsboLocation = 0;
	constexpr static GraphicInt k_lightClusterSizesSsboLocation = 1;
	constexpr static GraphicInt k_lightClusterIndicesSsboLocation = 2;

	struct alignas(16) ViewParams
	{
		glm::mat4 worldToView;
		glm::mat4 viewToProjected;
		glm::mat4 worldToProjected;
		glm::mat4 projectedToView;
		glm::vec3 viewPosition;
	};

	struct alignas(16) LightParams
	{
		glm::ivec2 lightClusterSizes;
		glm::ivec2 resolution;
		float near;
		float far;
		int32_t lightClusterCountZ;
		int32_t maxLightCountPerCluster;
		int32_t totalLightCount;
	};

	struct alignas(16) ModelParams
	{
		glm::mat4 modelToWorld;
	};

	struct alignas(16) RigParams
	{
		std::array<glm::mat4, 100> bones;
	};

	struct alignas(16) Light
	{
		// xyz is position, w is radius
		glm::vec4 positionAndRadius;
		// xyz is color, w is intensity
		glm::vec4 colorAndIntensity;
		// xyz is direction, w is type (0 = point, 1 = spot, TODO: directional)
		glm::vec4 directionAndType;
		// x is outer radius, y is inner radius
		glm::vec4 spotCosAngles;
	};

	struct DebugParams
	{
		DebugMode::Type mode;
	};

}
