#include <vob/aoe/rendering/ProgramUtils.h>

#include <vob/aoe/debug/Check.h>
#include <vob/aoe/rendering/MaterialParamsLayout.h>

#include <vob/misc/std/ignorable_assert.h>

#include <filesystem>
#include <optional>
#include <span>

#define AOEGL_DEBUG 1
#if !defined(NDEBUG) || defined(AOEGL_DEBUG)
#include <iomanip>
#include <iostream>
#include <vector>
#endif

#ifndef VOB_AOEGL_SHADER_DIR
#define VOB_AOEGL_SHADER_DIR "data/shaders/"
#endif


namespace vob::aoegl
{
#if !defined(NDEBUG) || defined(AOEGL_DEBUG)
	void debugPrintSource(std::string_view a_source)
	{
		auto i = 0;
		while (!a_source.empty())
		{
			auto const pos = a_source.find('\n');
			std::cout << std::setw(3) << (i++) << ' ' << a_source.substr(0, pos) << '\n';
			a_source = a_source.substr(pos + 1);
		}
		std::cout.flush();
	}
#endif

	bool tryExportCoreShaders()
	{
#if defined(VOB_AOEGL_SHADER_EXPORTER) && defined(VOB_AOEGL_SHADER_SOURCE_DIR)
		auto const exporter = std::filesystem::path{VOB_AOEGL_SHADER_EXPORTER}.make_preferred();
		auto const sourceDir = std::filesystem::path{VOB_AOEGL_SHADER_SOURCE_DIR}.make_preferred();
		auto const destinationDir = std::filesystem::path{VOB_AOEGL_SHADER_DIR "core"}.make_preferred();

		auto command = std::string{'"'};
		command += '"' + exporter.string() + "\" \"" + sourceDir.string() + "\" \"" + destinationDir.string() + '"';
		command += '"';

		auto const result = std::system(command.c_str());
		return VOB_AOE_CHECK_LOG(result == 0, "Core shader export failed ({}).", result);
#else
		return false;
#endif
	}

	GraphicId createShader(GraphicEnum a_shaderType, std::string_view a_shaderSource)
	{
		auto const shaderId = glCreateShader(a_shaderType);
		auto const shaderSourceCStr = a_shaderSource.data();
		auto const shaderSourceSize = static_cast<GraphicInt>(a_shaderSource.size());

		glShaderSource(shaderId, 1, &shaderSourceCStr, &shaderSourceSize);
		glCompileShader(shaderId);
		GraphicInt compilationStatus;
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compilationStatus);
		if (compilationStatus != GL_TRUE)
		{
#if !defined(NDEBUG) || defined(AOEGL_DEBUG)
			GraphicInt errorLogLength;
			glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &errorLogLength);
			std::vector<char> rawErrorLog;
			rawErrorLog.resize(errorLogLength);
			glGetShaderInfoLog(shaderId, errorLogLength, &errorLogLength, rawErrorLog.data());
			std::string_view errorLog{rawErrorLog.data(), rawErrorLog.size()};
			std::cerr << errorLog << std::endl;
			debugPrintSource(a_shaderSource);
			std::cerr << errorLog << std::endl;
#endif
			ignorable_assert(false && "Shader compilation failed.");
			glDeleteShader(shaderId);
			return k_invalidId;
		}

		return shaderId;
	}

	GraphicId createProgram(
		std::string_view a_vertexShaderSource, std::string_view a_fragmentShaderSource, GraphicId a_optionalProgramId)
	{
		auto const vertexShaderId = createShader(GL_VERTEX_SHADER, a_vertexShaderSource);
		if (vertexShaderId == k_invalidId)
		{
			return k_invalidId;
		}

		auto const fragmentShaderId = createShader(GL_FRAGMENT_SHADER, a_fragmentShaderSource);
		if (fragmentShaderId == k_invalidId)
		{
			glDeleteShader(vertexShaderId);
			return k_invalidId;
		}

		auto const programId = a_optionalProgramId != k_invalidId ? a_optionalProgramId : glCreateProgram();
		glAttachShader(programId, vertexShaderId);
		glAttachShader(programId, fragmentShaderId);
		glLinkProgram(programId);
		glDetachShader(programId, vertexShaderId);
		glDetachShader(programId, fragmentShaderId);
		glDeleteShader(vertexShaderId);
		glDeleteShader(fragmentShaderId);

		GraphicInt linkStatus;
		glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE)
		{
#if !defined(NDEBUG) || defined(AOEGL_DEBUG)
			GraphicInt errorLogLength;
			glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &errorLogLength);
			std::vector<char> rawErrorLog;
			rawErrorLog.resize(errorLogLength);
			glGetProgramInfoLog(programId, errorLogLength, &errorLogLength, rawErrorLog.data());
			std::string_view errorLog{rawErrorLog.data(), rawErrorLog.size()};
			std::cerr << errorLog << std::endl;
#endif
			ignorable_assert(false && "Program linking failed.");
			glDeleteProgram(programId);
			return k_invalidId;
		}

		return programId;
	}

	namespace
	{
		GraphicId createComputeProgram(
			std::string_view a_computeShaderSource, GraphicId a_optionalProgramId = k_invalidId)
		{
			auto const computeShaderId = createShader(GL_COMPUTE_SHADER, a_computeShaderSource);
			if (computeShaderId == k_invalidId)
			{
				return k_invalidId;
			}

			auto const programId = a_optionalProgramId != k_invalidId ? a_optionalProgramId : glCreateProgram();
			glAttachShader(programId, computeShaderId);
			glLinkProgram(programId);
			glDetachShader(programId, computeShaderId);
			glDeleteShader(computeShaderId);

			GraphicInt linkStatus;
			glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
			if (linkStatus != GL_TRUE)
			{
#if !defined(NDEBUG) || defined(AOEGL_DEBUG)
				GraphicInt errorLogLength;
				glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &errorLogLength);
				std::vector<char> rawErrorLog;
				rawErrorLog.resize(errorLogLength);
				glGetProgramInfoLog(programId, errorLogLength, &errorLogLength, rawErrorLog.data());
				std::string_view errorLog{rawErrorLog.data(), rawErrorLog.size()};
				std::cerr << errorLog << std::endl;
				debugPrintSource(a_computeShaderSource);
#endif
				ignorable_assert(false && "Program linking failed.");
				glDeleteProgram(programId);
				return k_invalidId;
			}

			return programId;
		}

		void setDefines(std::string& a_source, std::span<std::string const> a_defines)
		{
			if (a_defines.empty())
			{
				return;
			}

			auto definesSourceSize = size_t{0};
			for (auto const& define : a_defines)
			{
				definesSourceSize += std::char_traits<char>::length("#define ") + define.size() + 1;
			}
			a_source.reserve(a_source.size() + definesSourceSize);

			std::string definesSource;
			definesSource.reserve(definesSourceSize);
			for (auto const& define : a_defines)
			{
				definesSource += "#define ";
				definesSource += define;
				definesSource += "\n";
			}

			auto const versionBegin = a_source.find("#version");
			auto const versionEnd = a_source.find("\n", versionBegin);
			a_source.insert(versionEnd + 1, definesSource);
		}
	}

	GraphicId createLightClusteringProgram(CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId)
	{
		return createComputeProgram(a_coreShaderSources.lightClusteringShader->source, a_optionalProgramId);
	}

	GraphicId createGeometryProgram(
		CoreShaderSources const& a_coreShaderSources,
		std::string_view a_fragmentShaderSource,
		ModelType a_modelType,
		bool a_useShading,
		bool a_useUv,
		bool a_useNormal,
		std::span<std::string const> a_extraDefines,
		GraphicId a_optionalProgramId)
	{
		std::vector<std::string> defines{a_extraDefines.begin(), a_extraDefines.end()};
		switch (a_modelType)
		{
		case ModelType::Rigged:
			defines.emplace_back("USE_RIG 1");
			break;
		case ModelType::Instanced:
			defines.emplace_back("USE_INSTANCING 1");
			break;
		default:
			break;
		}
		if (a_useShading)
		{
			defines.emplace_back("USE_SHADING 1");
		}
		if (a_useUv)
		{
			defines.emplace_back("USE_UV 1");
		}
		if (a_useNormal)
		{
			defines.emplace_back("USE_NORMAL 1");
		}

		auto vertexShaderSource = a_coreShaderSources.geometryVertexShader->source;
		setDefines(vertexShaderSource, defines);

		auto fragmentShaderSource = std::string{a_fragmentShaderSource};
		setDefines(fragmentShaderSource, defines);
		return createProgram(vertexShaderSource, fragmentShaderSource, a_optionalProgramId);
	}

	GraphicId createShadingProgram(
		CoreShaderSources const& a_coreShaderSources,
		std::string_view a_partialSource,
		std::span<std::string const> a_defines,
		MaterialParamsLayout const& a_paramsLayout,
		ShadingPass a_shadingPass,
		ModelType a_modelType,
		bool a_isAlphaMasked,
		GraphicId a_optionalProgramId)
	{
		auto const& shellSource = [&a_coreShaderSources, a_shadingPass]() -> std::string const&
		{
			switch (a_shadingPass)
			{
			case ShadingPass::Opaque:
				return a_coreShaderSources.opaqueShell->source;
			default:
				ignorable_assert(false && "No shell for this shading pass yet.");
				return a_coreShaderSources.opaqueShell->source;
			}
		}();

		auto fragmentShaderSource = shellSource;
		fragmentShaderSource += '\n';
		fragmentShaderSource += generateMaterialParamsBlockSource(a_paramsLayout);
		fragmentShaderSource += '\n';
		fragmentShaderSource += a_partialSource;

		auto defines = std::vector<std::string>{a_defines.begin(), a_defines.end()};
		if (a_isAlphaMasked)
		{
			defines.emplace_back("USE_ALPHA_MASK 1");
		}

		return createGeometryProgram(
			a_coreShaderSources,
			fragmentShaderSource,
			a_modelType,
			true /* use shading */,
			true /* use uv */,
			false /* use normal */,
			defines,
			a_optionalProgramId);
	}

	namespace
	{
		GraphicId createAlphaMaskedGeometryProgram(
			CoreShaderSources const& a_coreShaderSources,
			std::string_view a_shellSource,
			std::string_view a_partialSource,
			std::span<std::string const> a_defines,
			MaterialParamsLayout const& a_paramsLayout,
			ModelType a_modelType,
			bool a_useNormal,
			GraphicId a_optionalProgramId)
		{
			auto fragmentShaderSource = std::string{a_shellSource};
			fragmentShaderSource += '\n';
			fragmentShaderSource += generateMaterialParamsBlockSource(a_paramsLayout);
			fragmentShaderSource += '\n';
			fragmentShaderSource += a_partialSource;

			auto defines = std::vector<std::string>{a_defines.begin(), a_defines.end()};
			defines.emplace_back("USE_ALPHA_MASK 1");

			return createGeometryProgram(
				a_coreShaderSources,
				fragmentShaderSource,
				a_modelType,
				false /* use shading */,
				true /* use uv */,
				a_useNormal,
				defines,
				a_optionalProgramId);
		}
	}

	GraphicId createDepthProgram(
		CoreShaderSources const& a_coreShaderSources, ModelType a_modelType, GraphicId a_optionalProgramId)
	{
		return createGeometryProgram(
			a_coreShaderSources,
			a_coreShaderSources.depthShell->source,
			a_modelType,
			false /* use shading */,
			false /* use uv */,
			true /* use normal */,
			{} /* extra defines */,
			a_optionalProgramId);
	}

	GraphicId createAlphaMaskedDepthProgram(
		CoreShaderSources const& a_coreShaderSources,
		std::string_view a_partialSource,
		std::span<std::string const> a_defines,
		MaterialParamsLayout const& a_paramsLayout,
		ModelType a_modelType,
		GraphicId a_optionalProgramId)
	{
		return createAlphaMaskedGeometryProgram(
			a_coreShaderSources,
			a_coreShaderSources.depthShell->source,
			a_partialSource,
			a_defines,
			a_paramsLayout,
			a_modelType,
			true /* use normal */,
			a_optionalProgramId);
	}

	GraphicId createShadowMapProgram(
		CoreShaderSources const& a_coreShaderSources, ModelType a_modelType, GraphicId a_optionalProgramId)
	{
		return createGeometryProgram(
			a_coreShaderSources,
			a_coreShaderSources.shadowMapShell->source,
			a_modelType,
			false /* use shading */,
			false /* use uv */,
			false /* use normal */,
			{} /* extra defines */,
			a_optionalProgramId);
	}

	GraphicId createAlphaMaskedShadowMapProgram(
		CoreShaderSources const& a_coreShaderSources,
		std::string_view a_partialSource,
		std::span<std::string const> a_defines,
		MaterialParamsLayout const& a_paramsLayout,
		ModelType a_modelType,
		GraphicId a_optionalProgramId)
	{
		return createAlphaMaskedGeometryProgram(
			a_coreShaderSources,
			a_coreShaderSources.shadowMapShell->source,
			a_partialSource,
			a_defines,
			a_paramsLayout,
			a_modelType,
			false /* use normal */,
			a_optionalProgramId);
	}

	GraphicId createQuadProgram(
		CoreShaderSources const& a_coreShaderSources,
		std::string_view a_fragmentShaderSource,
		GraphicId a_optionalProgramId)
	{
		return createProgram(
			a_coreShaderSources.quadVertexShader->source, a_fragmentShaderSource, a_optionalProgramId);
	}

	GraphicId createSsaoDepthProgram(CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId)
	{
		return createQuadProgram(
			a_coreShaderSources, a_coreShaderSources.ssaoDepthFragmentShader->source, a_optionalProgramId);
	}

	GraphicId createSsaoProgram(CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId)
	{
		return createQuadProgram(
			a_coreShaderSources, a_coreShaderSources.ssaoFragmentShader->source, a_optionalProgramId);
	}

	GraphicId createSsaoBlurProgram(CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId)
	{
		return createQuadProgram(
			a_coreShaderSources, a_coreShaderSources.ssaoBlurFragmentShader->source, a_optionalProgramId);
	}

	GraphicId createSkyProgram(
		CoreShaderSources const& a_coreShaderSources,
		std::string_view a_skyPartialSource,
		GraphicId a_optionalProgramId)
	{
		auto fragmentShaderSource = a_coreShaderSources.skyShell->source;
		fragmentShaderSource += '\n';
		fragmentShaderSource += a_skyPartialSource;
		return createQuadProgram(a_coreShaderSources, fragmentShaderSource, a_optionalProgramId);
	}

	GraphicId createSkyIrradianceProgram(
		CoreShaderSources const& a_coreShaderSources,
		std::string_view a_skyPartialSource,
		GraphicId a_optionalProgramId)
	{
		auto computeShaderSource = a_coreShaderSources.skyIrradianceShader->source;
		computeShaderSource += '\n';
		computeShaderSource += a_skyPartialSource;
		return createComputeProgram(computeShaderSource, a_optionalProgramId);
	}

	GraphicId createSsrProgram(
		CoreShaderSources const& a_coreShaderSources,
		std::string_view a_skyPartialSource,
		GraphicId a_optionalProgramId)
	{
		auto fragmentShaderSource = a_coreShaderSources.ssrFragmentShader->source;
		fragmentShaderSource += '\n';
		fragmentShaderSource += a_skyPartialSource;
		return createQuadProgram(a_coreShaderSources, fragmentShaderSource, a_optionalProgramId);
	}

	GraphicId createHiZReduceProgram(CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId)
	{
		return createQuadProgram(a_coreShaderSources, a_coreShaderSources.hizReduce->source, a_optionalProgramId);
	}

	GraphicId createSsrPrefilterProgram(CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId)
	{
		return createQuadProgram(
			a_coreShaderSources, a_coreShaderSources.ssrPrefilter->source, a_optionalProgramId);
	}

	GraphicId createSsrDownsampleProgram(CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId)
	{
		return createQuadProgram(
			a_coreShaderSources, a_coreShaderSources.ssrDownsample->source, a_optionalProgramId);
	}

	GraphicId createOpaqueCompositionProgram(
		CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId)
	{
		return createQuadProgram(
			a_coreShaderSources, a_coreShaderSources.opaqueCompositionFragmentShader->source, a_optionalProgramId);
	}

	GraphicId createDebugProgram(CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId)
	{
		return createQuadProgram(
			a_coreShaderSources, a_coreShaderSources.debugFragmentShader->source, a_optionalProgramId);
	}

	GraphicId createDebugGeometryProgram(CoreShaderSources const& a_coreShaderSources, GraphicId a_optionalProgramId)
	{
		return createProgram(
			a_coreShaderSources.debugGeometryVertexShader->source,
			a_coreShaderSources.debugGeometryFragmentShader->source,
			a_optionalProgramId);
	}
}
