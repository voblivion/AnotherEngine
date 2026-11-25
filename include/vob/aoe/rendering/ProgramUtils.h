#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/CoreShaderSources.h>
#include <vob/aoe/rendering/GraphicTypes.h>
#include <vob/aoe/rendering/MaterialParamsLayout.h>
#include <vob/aoe/rendering/ShadingPass.h>

#include <span>
#include <string>
#include <string_view>


namespace vob::aoegl
{
	bool tryExportCoreShaders();

	GraphicId createProgram(
		std::string_view a_vertexShaderSource,
		std::string_view a_fragmentShaderSource,
		GraphicId optionalProgramId = k_invalidId);

	GraphicId createLightClusteringProgram(
		CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId = k_invalidId);

	enum class ModelType
	{
		Static,
		Rigged,
		Instanced
	};

	// Appends a material partial to the pass shell; the shell forward-declares what the partial defines.
	GraphicId createShadingProgram(
		CoreShaderSources const& a_coreShaderSources,
		std::string_view a_partialSource,
		std::span<std::string const> a_defines,
		MaterialParamsLayout const& a_paramsLayout,
		ShadingPass a_shadingPass,
		ModelType a_modelType,
		bool a_isAlphaMasked,
		GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createDepthProgram(
		CoreShaderSources const& a_coreShaderSources,
		ModelType a_modelType,
		GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createAlphaMaskedDepthProgram(
		CoreShaderSources const& a_coreShaderSources,
		std::string_view a_partialSource,
		std::span<std::string const> a_defines,
		MaterialParamsLayout const& a_paramsLayout,
		ModelType a_modelType,
		GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createShadowMapProgram(
		CoreShaderSources const& a_coreShaderSources,
		ModelType a_modelType,
		GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createAlphaMaskedShadowMapProgram(
		CoreShaderSources const& a_coreShaderSources,
		std::string_view a_partialSource,
		std::span<std::string const> a_defines,
		MaterialParamsLayout const& a_paramsLayout,
		ModelType a_modelType,
		GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createQuadProgram(
		CoreShaderSources const& a_coreShaderSources,
		std::string_view a_fragmentShaderSource,
		GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createSsaoDepthProgram(
		CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createSsaoProgram(
		CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createSsaoBlurProgram(
		CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createSsrProgram(
		CoreShaderSources const& a_coreShaderSources,
		std::string_view a_skyPartialSource,
		GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createHiZReduceProgram(
		CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createSsrPrefilterProgram(
		CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createSsrDownsampleProgram(
		CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createSkyProgram(
		CoreShaderSources const& a_coreShaderSources,
		std::string_view a_skyPartialSource,
		GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createSkyIrradianceProgram(
		CoreShaderSources const& a_coreShaderSources,
		std::string_view a_skyPartialSource,
		GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createOpaqueCompositionProgram(
		CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createDebugProgram(
		CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId = k_invalidId);

	GraphicId createDebugGeometryProgram(
		CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId = k_invalidId);
}
