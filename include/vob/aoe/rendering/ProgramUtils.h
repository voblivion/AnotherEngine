#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/GraphicTypes.h>

#include <vob/aoe/rendering/Program.h>
#include <vob/aoe/rendering/ProgramData.h>


namespace vob::aoegl
{
	// TODO: very old, remove please
	void createProgram(ProgramData const& a_programData, DebugProgram& a_program);

	GraphicId createProgram(
		std::string_view a_vertexShaderSource, std::string_view a_fragmentShaderSource, GraphicId optionalProgramId = k_invalidId);

	// TODO: somewhat old, remove
	GraphicId oldCreateLightClusteringProgram(int32_t a_workGroupSize = 128);

	GraphicId oldCreateDepthProgram(bool a_useRig);

	GraphicId oldCreateForwardProgram(
		std::string_view a_shadingSource, bool a_useRig, GraphicId optionalProgramId = k_invalidId);

	GraphicId oldCreatePostProcessProgram(
		std::string_view a_postProcessSource, GraphicId optionalProgramId = k_invalidId);

	// TODO: remove what's above
	GraphicId createLightClusteringProgram(int32_t a_workGroupSize, GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createShadingProgram(std::string_view a_fragmentShaderSource, bool a_useRig, GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createDepthProgram(bool a_useRig, GraphicId a_optionalProgramId = k_invalidId);
	
	GraphicId createShadowMapProgram(bool a_useRig, GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createQuadProgram(std::string_view a_fragmentShaderSource, GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createSsaoProgram(GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createSsrProgram(GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createOpaqueCompositionProgram(GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createDebugProgram(GraphicId a_optionalProgramId = k_invalidId);
}
