#pragma once

#include "vob/aoe/rendering/shaders/defines.h"

#include "vob/misc/std/bounded_vector.h"

#include "glm/glm.hpp"

#include <cstdint>


namespace vob::aoegl
{
	struct RenderSceneConfig
	{
		struct Lighting
		{
			int32_t maxLightCount = 2048;
			glm::ivec2 clusterTileSize = glm::ivec2{ 16 };
			int32_t clusterZCount = 24;
			int32_t clusterCapacity = 64;
		};

		struct Shadow
		{
			bool isSunEnabled = true;
			glm::ivec2 sunResolution = glm::ivec2{ 4096 };
			mistd::bounded_vector<float, k_sunCascadingShadowMapsCapacity> sunCascadeFarClips = { 10.0f, 40.0f, 200.0f, 1000.0f };

			int32_t spotCount = 12;
			glm::ivec2 spotMaxResolution = glm::ivec2{ 2048 };
			glm::ivec2 spotMinResolution = glm::ivec2{ 256 };
			int32_t spotBucketSize = 4;
		};

		struct Ssao
		{
			bool isEnabled = true;
			float scale = 1.0f;
			int32_t sliceCount = 3;
			int32_t stepCount = 8;
			float radius = 1.0f;
			float falloffStart = 0.7f;
			float intensity = 1.0f;
			float maxRadiusScreenFraction = 0.2f;
			float depthTolerance = 0.05f;
		};

		struct Ssr
		{
			bool isEnabled = true;
			float scale = 1.0f;
			int32_t stepCount = 64;
			float maxRange = 500.0f;
			float penetrationBlockedRatio = 0.003f;
			float penetrationThroughRatio = 0.3f;

			// TEMPORARY
			bool debugExitReason = false;
			bool debugRay = false;
			bool debugPenetration = false;
			glm::ivec2 debugRayPixel = glm::ivec2{ 0 };
		};

		struct Bloom
		{
			bool isEnabled = true;
			float scale = 0.5f;
			float spreadScreenFraction = 0.05f;
			float filterRadius = 1.0f;
			float scatter = 0.5f;
			float strength = 0.05f;
			bool useKarisAverage = true;
		};

		struct Tonemap
		{
			glm::vec3 colorFilter = glm::vec3{ 1.0f };
			float exposure = 1.0f;
			float contrast = 1.0f;
			float saturation = 1.0f;
		};

		float renderScale = 1.0f;
		Lighting lighting;
		Shadow shadow;
		Ssao ssao;
		Ssr ssr;
		Bloom bloom;
		Tonemap tonemap;

		// maybe later
		// int32_t maxFrameRate
		// int32_t inFlightFrameCount
		// int32_t anisotropy
	};
}
