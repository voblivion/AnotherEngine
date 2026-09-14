#pragma once

#include "vob/aoe/rendering/RenderSceneConfig.h"
#include "vob/aoe/rendering/resources/GpuDeleteQueue.h"
#include "vob/aoe/rendering/resources/GpuRenderSceneResources.h"

#include "glm/glm.hpp"

#include <string_view>


namespace vob::aoegl
{
	struct RenderSceneProgramSources
	{
		std::string_view skyPartial;
		std::string_view bloomDownsample;
		std::string_view bloomUpsample;
		std::string_view bloomCombine;
		std::string_view tonemap;
		std::string_view antiAliasing;
		std::string_view present;
	};

	GpuRenderSceneResources createGpuRenderSceneResources(
		RenderSceneConfig const& a_config,
		glm::ivec2 a_windowSize,
		RenderSceneProgramSources const& a_programSources,
		GpuDeleteQueue& a_deleteQueue);

	void updateGpuRenderSceneResources(
		RenderSceneConfig const& a_appliedConfig,
		glm::ivec2 a_appliedWindowSize,
		RenderSceneConfig const& a_config,
		glm::ivec2 a_windowSize,
		GpuDeleteQueue& a_deleteQueue,
		GpuRenderSceneResources& o_resources);
}
