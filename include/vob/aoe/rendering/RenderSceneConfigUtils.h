#pragma once

#include "vob/aoe/rendering/RenderSceneConfig.h"

#include <algorithm>
#include <cmath>
#include <cstdint>


namespace vob::aoegl
{
	inline int32_t computeBloomMinMipWidth(RenderSceneConfig::Bloom const& a_config)
	{
		return static_cast<int32_t>(std::round(1.0f / std::max(a_config.spreadScreenFraction, 1e-4f)));
	}

	inline int32_t computeSpotShadowMapCount(RenderSceneConfig::Shadow const& a_config)
	{
		return std::clamp(a_config.spotCount, 0, k_spotLightShadowMapsCapacity);
	}

	inline glm::ivec2 computeSpotShadowMapResolution(
		RenderSceneConfig::Shadow const& a_config
		, int32_t a_slotIndex)
	{
		auto const bucketSize = std::max(a_config.spotBucketSize, 1);
		auto const shift = a_slotIndex / bucketSize;
		return glm::max(a_config.spotMaxResolution >> glm::ivec2{ shift }, a_config.spotMinResolution);
	}

	inline UniformSsaoParams createUniformSsaoParams(RenderSceneConfig::Ssao const& a_config)
	{
		return UniformSsaoParams{
			.sliceCount = a_config.sliceCount,
			.stepCount = a_config.stepCount,
			.radius = a_config.radius,
			.falloffStart = a_config.falloffStart,
			.intensity = a_config.intensity,
			.maxRadiusScreenFraction = a_config.maxRadiusScreenFraction,
			.depthTolerance = a_config.depthTolerance
		};
	}

	inline UniformSsrParams createUniformSsrParams(RenderSceneConfig::Ssr const& a_config)
	{
		return UniformSsrParams{
			.stepCount = a_config.stepCount,
			.debugExitReason = a_config.debugExitReason ? 1 : 0,
			.maxRange = a_config.maxRange,
			.debugRay = a_config.debugRay ? 1 : 0,
			.debugRayPixel = a_config.debugRayPixel,
			.penetrationBlockedRatio = a_config.penetrationBlockedRatio,
			.penetrationThroughRatio = a_config.penetrationThroughRatio,
			.debugPenetration = a_config.debugPenetration ? 1 : 0,
			.isEnabled = a_config.isEnabled ? 1 : 0
		};
	}

	inline UniformBloomParams createUniformBloomParams(
		RenderSceneConfig::Bloom const& a_config
		, int32_t a_mipCount)
	{
		auto const mipCount = static_cast<float>(a_mipCount);
		return UniformBloomParams{
			.filterRadius = a_config.filterRadius,
			.scatter = a_config.scatter,
			.strength = a_config.strength,
			.useKarisAverage = a_config.useKarisAverage ? 1 : 0,
			.totalWeight = std::abs(1.0f - a_config.scatter) < 1e-4f
				? mipCount
				: (1.0f - std::pow(a_config.scatter, mipCount)) / (1.0f - a_config.scatter)
		};
	}

	inline UniformTonemapParams createUniformTonemapParams(RenderSceneConfig::Tonemap const& a_config)
	{
		return UniformTonemapParams{
			.colorFilter = a_config.colorFilter,
			.exposure = a_config.exposure,
			.contrast = a_config.contrast,
			.saturation = a_config.saturation
		};
	}
}
