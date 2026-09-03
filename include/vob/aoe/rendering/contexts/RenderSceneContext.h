#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"
#include "vob/aoe/rendering/RenderSceneConfig.h"
#include "vob/aoe/rendering/resources/GpuRenderSceneTargets.h"
#include "vob/aoe/rendering/shaders/defines.h"

#include <vob/misc/std/enum_traits.h>
#include <vob/misc/std/bounded_vector.h>
#include <vob/misc/std/enum_map.h>

#include <entt/entity/entity.hpp>

#include <glm/glm.hpp>

#include <chrono>
#include <functional>


namespace vob::aoegl
{
	struct RenderSceneContext
	{
		std::reference_wrapper<RenderSceneConfig> config;

		glm::vec3 sunDir = glm::normalize(glm::vec3{ 0.7f, 0.15f, -1.0f });
		struct SpotLightShadowFade
		{
			entt::entity entity = entt::null;
			float fade = 0.0f;
		};
		mistd::bounded_vector<SpotLightShadowFade, k_spotLightShadowMapsCapacity> spotLightShadowFades;

		GpuRenderSceneTargets targets;
	};
}
