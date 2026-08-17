#include <vob/aoe/rendering/ProgramUtils.h>

#include <vob/aoe/debug/Check.h>

#include <vob/misc/std/ignorable_assert.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <regex>
#include <span>
#include <sstream>

#define AOEGL_DEBUG 1
#if !defined(NDEBUG) || defined(AOEGL_DEBUG)
#include <vector>
#include <iomanip>
#include <iostream>
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
		auto const exporter = std::filesystem::path{ VOB_AOEGL_SHADER_EXPORTER }.make_preferred();
		auto const sourceDir = std::filesystem::path{ VOB_AOEGL_SHADER_SOURCE_DIR }.make_preferred();
		auto const destinationDir = std::filesystem::path{ VOB_AOEGL_SHADER_DIR "core" }.make_preferred();

		auto command = std::string{ '"' };
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
			std::string_view errorLog{ rawErrorLog.data(), rawErrorLog.size() };
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

	GraphicId createProgram(std::string_view a_vertexShaderSource, std::string_view a_fragmentShaderSource, GraphicId a_optionalProgramId)
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
			std::string_view errorLog{ rawErrorLog.data(), rawErrorLog.size() };
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
		GraphicId createComputeProgram(std::string_view a_computeShaderSource, GraphicId a_optionalProgramId = k_invalidId)
		{
			auto const computeShaderId = createShader(GL_COMPUTE_SHADER, a_computeShaderSource);
			if (computeShaderId == k_invalidId)
			{
				return k_invalidId;
			}

			auto const programId = a_optionalProgramId != k_invalidId ? a_optionalProgramId : glCreateProgram();
			glAttachShader(programId, computeShaderId);
			glLinkProgram(programId);
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
				std::string_view errorLog{ rawErrorLog.data(), rawErrorLog.size() };
				std::cerr << errorLog << std::endl;
				debugPrintSource(a_computeShaderSource);
#endif
				ignorable_assert(false && "Program linking failed.");
				glDeleteProgram(programId);
				return k_invalidId;
			}

			return programId;
		}

		std::string readFile(std::string const& a_fileName)
		{
			std::ifstream file(a_fileName);
			if (!file.is_open())
			{
#if !defined(NDEBUG) || defined(AOEGL_DEBUG)
				std::cerr << a_fileName << std::endl;
#endif
				ignorable_assert(false && "File not found.");
				return {};
			}

			std::stringstream buffer;
			buffer << file.rdbuf();
			return buffer.str();
		}

		void setDefines(std::string& a_source, std::span<std::string const> a_defines)
		{
			if (a_defines.empty())
			{
				return;
			}

			auto definesSourceSize = size_t{ 0 };
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

		void processIncludes(std::string& a_source)
		{
			auto const includeRegex = std::regex(R"(#include\s*["<](.*?)[">])");
			auto match = std::smatch{};
			auto searchStart(a_source.cbegin());

			while (std::regex_search(searchStart, a_source.cend(), match, includeRegex))
			{
				auto const includedFileName = std::string{ VOB_AOEGL_SHADER_DIR } + match[1].str();
				auto const includedFileContent = readFile(includedFileName);

				auto const matchPos = match.position(0) + (searchStart - a_source.cbegin());
				a_source.replace(matchPos, match.length(0), includedFileContent);

				searchStart = a_source.cbegin() + matchPos;
			}

			return;
		}

		std::string createVertexGeometryShaderSource(bool a_useRig, bool a_useShading)
		{
			std::vector<std::string> defines;
			if (a_useRig)
			{
				defines.emplace_back("USE_RIG 1");
			}
			if (a_useShading)
			{
				defines.emplace_back("USE_SHADING 1");
			}

			auto source = readFile(VOB_AOEGL_SHADER_DIR "core/geometry_vertex_shader.glsl");
			setDefines(source, defines);
			processIncludes(source);
			return source;
		}
	}

	GraphicId createLightClusteringProgram(int32_t a_workGroupSize, GraphicId a_optionalProgramId)
	{
		std::vector<std::string> defines;
		defines.emplace_back("WORK_GROUP_SIZE " + std::to_string(a_workGroupSize));

		auto computeShaderSource = readFile(VOB_AOEGL_SHADER_DIR "core/light_clustering_shader.glsl");
		setDefines(computeShaderSource, defines);
		processIncludes(computeShaderSource);
		return createComputeProgram(computeShaderSource, a_optionalProgramId);
	}

	GraphicId createGeometryProgram(
		std::string_view a_fragmentShaderSource
		, ModelType a_modelType
		, bool a_useShading
		, bool a_useNormal
		, std::span<std::string const> a_extraDefines
		, GraphicId a_optionalProgramId)
	{
		std::vector<std::string> defines{ a_extraDefines.begin(), a_extraDefines.end() };
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
		if (a_useNormal)
		{
			defines.emplace_back("USE_NORMAL 1");
		}

		auto vertexShaderSource = readFile(VOB_AOEGL_SHADER_DIR "core/geometry_vertex_shader.glsl");
		setDefines(vertexShaderSource, defines);
		processIncludes(vertexShaderSource);

		auto fragmentShaderSource = std::string{ a_fragmentShaderSource };
		setDefines(fragmentShaderSource, defines);
		processIncludes(fragmentShaderSource);
		return createProgram(vertexShaderSource, fragmentShaderSource, a_optionalProgramId);
	}

	GraphicId createShadingProgram(
		std::string_view a_partialSource
		, std::span<std::string const> a_defines
		, ShadingPass a_shadingPass
		, ModelType a_modelType
		, GraphicId a_optionalProgramId)
	{
		auto const shellPath = [a_shadingPass]
			{
				switch (a_shadingPass)
				{
				case ShadingPass::Opaque:
					return VOB_AOEGL_SHADER_DIR "core/opaque_shell.glsl";
				default:
					ignorable_assert(false && "No shell for this shading pass yet.");
					return VOB_AOEGL_SHADER_DIR "core/opaque_shell.glsl";
				}
			}();

		auto fragmentShaderSource = readFile(shellPath);
		fragmentShaderSource += '\n';
		fragmentShaderSource += a_partialSource;
		return createGeometryProgram(
			fragmentShaderSource, a_modelType, true /* use shading */, false /* use normal */, a_defines, a_optionalProgramId);
	}

	GraphicId createDepthProgram(ModelType a_modelType, GraphicId a_optionalProgramId)
	{
		auto const fragmentShaderSource = readFile(VOB_AOEGL_SHADER_DIR "core/depth_fragment_shader.glsl");
		return createGeometryProgram(
			fragmentShaderSource, a_modelType, false /* use shading */, true /* use normal */, {} /* extra defines */, a_optionalProgramId);
	}

	GraphicId createShadowMapProgram(ModelType a_modelType, GraphicId a_optionalProgramId)
	{
		auto const fragmentShaderSource = readFile(VOB_AOEGL_SHADER_DIR "core/shadow_map_fragment_shader.glsl");
		return createGeometryProgram(
			fragmentShaderSource, a_modelType, false /* use shading */, false /* use normal */, {} /* extra defines */, a_optionalProgramId);
	}

	GraphicId createQuadProgram(std::string_view a_fragmentShaderSource, GraphicId a_optionalProgramId)
	{
		auto vertexShaderSource = readFile(VOB_AOEGL_SHADER_DIR "core/quad_vertex_shader.glsl");

		auto fragmentShaderSource = std::string{ a_fragmentShaderSource };
		processIncludes(fragmentShaderSource);

		return createProgram(vertexShaderSource, fragmentShaderSource, a_optionalProgramId);
	}

	GraphicId createSsaoProgram(GraphicId a_optionalProgramId)
	{
		auto const fragmentShaderSource = readFile(VOB_AOEGL_SHADER_DIR "core/ssao_fragment_shader.glsl");
		return createQuadProgram(fragmentShaderSource, a_optionalProgramId);
	}

	GraphicId createSkyProgram(std::string_view a_skyPartialSource, GraphicId a_optionalProgramId)
	{
		auto fragmentShaderSource = readFile(VOB_AOEGL_SHADER_DIR "core/sky_shell.glsl");
		fragmentShaderSource += '\n';
		fragmentShaderSource += a_skyPartialSource;
		return createQuadProgram(fragmentShaderSource, a_optionalProgramId);
	}

	GraphicId createSkyIrradianceProgram(std::string_view a_skyPartialSource, GraphicId a_optionalProgramId)
	{
		auto computeShaderSource = readFile(VOB_AOEGL_SHADER_DIR "core/sky_irradiance_shader.glsl");
		computeShaderSource += '\n';
		computeShaderSource += a_skyPartialSource;
		processIncludes(computeShaderSource);
		return createComputeProgram(computeShaderSource, a_optionalProgramId);
	}

	GraphicId createSsrProgram(std::string_view a_skyPartialSource, GraphicId a_optionalProgramId)
	{
		auto fragmentShaderSource = readFile(VOB_AOEGL_SHADER_DIR "core/ssr_fragment_shader.glsl");
		fragmentShaderSource += '\n';
		fragmentShaderSource += a_skyPartialSource;
		return createQuadProgram(fragmentShaderSource, a_optionalProgramId);
	}

	GraphicId createOpaqueCompositionProgram(GraphicId a_optionalProgramId)
	{
		auto const fragmentShaderSource = readFile(VOB_AOEGL_SHADER_DIR "core/opaque_composition_fragment_shader.glsl");
		return createQuadProgram(fragmentShaderSource, a_optionalProgramId);
	}

	GraphicId createDebugProgram(GraphicId a_optionalProgramId)
	{
		auto const fragmentShaderSource = readFile(VOB_AOEGL_SHADER_DIR "core/debug_fragment_shader.glsl");
		return createQuadProgram(fragmentShaderSource, a_optionalProgramId);
	}

	GraphicId createDebugGeometryProgram(GraphicId a_optionalProgramId)
	{
		auto vertexShaderSource = readFile(VOB_AOEGL_SHADER_DIR "core/debug_geometry_vertex_shader.glsl");
		processIncludes(vertexShaderSource);

		auto fragmentShaderSource = readFile(VOB_AOEGL_SHADER_DIR "core/debug_geometry_fragment_shader.glsl");
		processIncludes(fragmentShaderSource);

		return createProgram(vertexShaderSource, fragmentShaderSource, a_optionalProgramId);
	}
}
