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
			ShadowSun,
			ShadowSpot0,
			ShadowSpot1,
			ShadowSpot2,
			ShadowSpot3,
			ShadowSpot4,
			ShadowSpot5,
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
	constexpr static GraphicInt k_globalParamsUboLocation = 4;
	constexpr static GraphicInt k_materialParamsUboLocation = 10;

	constexpr static GraphicInt k_sunShadowMapTextureIndex = 0;
	constexpr static GraphicInt k_spotLightShadowMapsFirstIndex = 1;
	constexpr static GraphicInt k_maxSpotLightShadowMapCount = 6;
	constexpr static GraphicInt k_materialTexturesFirstIndex = 10;

	constexpr static GraphicInt k_lightsSsboLocation = 0;
	constexpr static GraphicInt k_lightClusterSizesSsboLocation = 1;
	constexpr static GraphicInt k_lightClusterIndicesSsboLocation = 2;

	struct alignas(16) GlobalParams
	{
		float worldTime;
	};

	struct alignas(16) ViewParams
	{
		glm::mat4 worldToView;
		glm::mat4 viewToProjected;
		glm::mat4 worldToProjected;
		glm::mat4 projectedToView;
		glm::vec3 viewPosition;
	};

	struct alignas(16) ShadowParams
	{
		glm::mat4 worldToProjected;
		float nearPlane;
		float farPlane;
		float lightSize;
		float _unused1;
	};

	struct alignas(16) LightParams
	{
		ShadowParams sunShadowParams;
		std::array<ShadowParams, k_maxSpotLightShadowMapCount> spotLightShadowParams;
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

	inline bool operator==(ModelParams const& a_lhs, ModelParams const& a_rhs)
	{
		return a_lhs.modelToWorld == a_rhs.modelToWorld;
	}

	struct alignas(16) RigParams
	{
		std::array<glm::mat4, 100> bones;
	};

	struct alignas(16) Light
	{
		glm::vec3 position;
		float radius;
		glm::vec3 color;
		float intensity;
		glm::vec3 direction;
		// 0 = point, 1 = spot, TODO: directional?
		float type;
		float spotOuterAngleCos;
		float spotInnerAngleCos;
		int32_t spotShadowMapIndex;
		float _unused0;
	};

	struct DebugParams
	{
		DebugMode::Type mode;
	};

	struct alignas(16) FxaaParams
	{
		glm::vec2 screenSizeInv;
	};
}
