#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/GraphicTypes.h>

#include <string_view>


namespace vob::aoegl
{
	GraphicId createProgram(
		std::string_view a_vertexShaderSource, std::string_view a_fragmentShaderSource, GraphicId optionalProgramId = k_invalidId);

	GraphicId createLightClusteringProgram(int32_t a_workGroupSize, GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createShadingProgram(std::string_view a_fragmentShaderSource, bool a_useRig, GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createDepthProgram(bool a_useRig, GraphicId a_optionalProgramId = k_invalidId);
	
	GraphicId createShadowMapProgram(bool a_useRig, GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createQuadProgram(std::string_view a_fragmentShaderSource, GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createSsaoProgram(GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createSsrProgram(GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createOpaqueCompositionProgram(GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createDebugProgram(GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createDebugGeometryProgram(GraphicId a_optionalProgramId = k_invalidId);
}
