#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/GraphicTypes.h>

#include <vob/aoe/rendering/Program.h>
#include <vob/aoe/rendering/ProgramData.h>


namespace vob::aoegl
{
	VOB_AOE_API GraphicId createProgram(std::string_view a_vertexShaderSource, std::string_view a_fragmentShaderSource);

	VOB_AOE_API GraphicId createComputeProgram(std::string_view a_computeShaderSource);

	VOB_AOE_API void createProgram(ProgramData const& a_programData, DebugProgram& a_program);

	VOB_AOE_API void createProgram(ProgramData const& a_programData, PostProcessProgram& a_program);

	VOB_AOE_API GraphicId createLightClusteringProgram(int32_t a_workGroupSize = 128);

	VOB_AOE_API GraphicId createDepthProgram(bool a_useRig);

	VOB_AOE_API GraphicId createForwardProgram(std::string_view a_shadingSource, bool a_useRig);

	VOB_AOE_API GraphicId createDebugForwardProgram(bool a_useRig);
}
