#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/GraphicTypes.h>

#include <string_view>


namespace vob::aoegl
{
	GraphicId createProgram(
		std::string_view a_vertexShaderSource, std::string_view a_fragmentShaderSource, GraphicId optionalProgramId = k_invalidId);

	GraphicId createLightClusteringProgram(int32_t a_workGroupSize, GraphicId a_optionalProgramId = k_invalidId);

	enum class ModelType
	{
		Static,
		Rigged,
		Instanced
	};

	GraphicId createShadingProgram(std::string_view a_fragmentShaderSource, ModelType a_modelType, GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createDepthProgram(ModelType a_modelType, GraphicId a_optionalProgramId = k_invalidId);
	
	GraphicId createShadowMapProgram(ModelType a_modelType, GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createQuadProgram(std::string_view a_fragmentShaderSource, GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createSsaoProgram(GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createSsrProgram(GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createOpaqueCompositionProgram(GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createDebugProgram(GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createDebugGeometryProgram(GraphicId a_optionalProgramId = k_invalidId);
}
