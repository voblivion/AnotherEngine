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
}
