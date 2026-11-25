#pragma once

#include "vob/aoe/rendering/CoreShaderSources.h"
#include "vob/aoe/rendering/RenderSceneConfig.h"
#include "vob/aoe/rendering/resources/GpuDeleteQueue.h"
#include "vob/aoe/rendering/resources/GpuRenderSceneResources.h"

#include "glm/glm.hpp"

#include <string_view>


namespace vob::aoegl
{
	GpuRenderSceneResources createGpuRenderSceneResources(
		RenderSceneConfig const& a_config,
		glm::ivec2 a_windowSize,
		CoreShaderSources const& a_coreShaderSources,
		std::string_view a_skyPartialSource,
		GpuDeleteQueue& a_deleteQueue);

	void updateGpuRenderSceneResources(
		RenderSceneConfig const& a_appliedConfig,
		glm::ivec2 a_appliedWindowSize,
		RenderSceneConfig const& a_config,
		glm::ivec2 a_windowSize,
		GpuDeleteQueue& a_deleteQueue,
		GpuRenderSceneResources& o_resources);
}
