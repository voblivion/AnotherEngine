#include <vob/aoe/rendering/ProgramUtils.h>

#include <vob/misc/std/ignorable_assert.h>

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
#ifndef VOB_AOEGL_CORE_SHADER_DIR_OLD
#define VOB_AOEGL_CORE_SHADER_DIR_OLD "data/shaders/old/"
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

	void createProgram(ProgramData const& a_programData, Program& a_program)
	{
		a_program.id = k_invalidId;

		if (a_programData.vertexShaderSource == nullptr
			|| a_programData.fragmentShaderSource == nullptr)
		{
			ignorable_assert(false && "Missing shader source");
			return;
		}

		auto const vertexShaderId = createShader(GL_VERTEX_SHADER, *a_programData.vertexShaderSource);
		if (vertexShaderId == k_invalidId)
		{
			return;
		}

		auto const fragmentShaderId = createShader(GL_FRAGMENT_SHADER, *a_programData.fragmentShaderSource);
		if (fragmentShaderId == k_invalidId)
		{
			glDeleteShader(vertexShaderId);
			return;
		}

		a_program.id = glCreateProgram();
		glAttachShader(a_program.id, vertexShaderId);
		glAttachShader(a_program.id, fragmentShaderId);
		glLinkProgram(a_program.id);
		glDeleteShader(vertexShaderId);
		glDeleteShader(fragmentShaderId);
		GraphicInt linkStatus;
		glGetProgramiv(a_program.id, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE)
		{
#ifndef NDEBUG
			GraphicInt errorLogLength;
			glGetProgramiv(a_program.id, GL_INFO_LOG_LENGTH, &errorLogLength);
			std::vector<char> rawErrorLog;
			rawErrorLog.resize(errorLogLength);
			glGetProgramInfoLog(
				a_program.id, errorLogLength, &errorLogLength, rawErrorLog.data());
			std::string_view errorLog{
				rawErrorLog.data(), static_cast<std::size_t>(errorLogLength) };
			std::cerr << errorLog << std::endl;
#endif
			a_program.id = 0;
		}
		glUseProgram(a_program.id);
	}

	void createProgram(ProgramData const& a_programData, SceneProgram& a_program)
	{
		createProgram(a_programData, static_cast<Program&>(a_program));
		if (a_program.id == k_invalidId)
		{
			return;
		}

		a_program.viewPositionLocation = glGetUniformLocation(a_program.id, "u_viewPosition");
		a_program.viewProjectionTransformLocation = glGetUniformLocation(a_program.id, "u_viewProjectionTransform");
	}

	void createProgram(ProgramData const& a_programData, DebugProgram& a_program)
	{
		createProgram(a_programData, static_cast<SceneProgram&>(a_program));
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

		void setDefines(std::string& a_source, std::span<std::pair<std::string_view, std::string_view>> a_defines)
		{
			if (a_defines.empty())
			{
				return;
			}

			auto definesSourceSize = size_t{ 0 };
			for (auto const& define : a_defines)
			{
				definesSourceSize += std::char_traits<char>::length("#define ") + define.first.size() + 1 + define.second.size() + 1;
			}
			a_source.reserve(a_source.size() + definesSourceSize);

			std::string definesSource;
			definesSource.reserve(definesSourceSize);
			for (auto const& define : a_defines)
			{
				definesSource += "#define ";
				definesSource += define.first;
				definesSource += " ";
				definesSource += define.second;
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

		void oldProcessIncludes(std::string& a_source)
		{
			auto const includeRegex = std::regex(R"(#include\s*["<](.*?)[">])");
			auto match = std::smatch{};
			auto searchStart(a_source.cbegin());

			while (std::regex_search(searchStart, a_source.cend(), match, includeRegex))
			{
				auto const includedFileName = std::string{ VOB_AOEGL_CORE_SHADER_DIR_OLD } + match[1].str();
				auto const includedFileContent = readFile(includedFileName);

				auto const matchPos = match.position(0) + (searchStart - a_source.cbegin());
				a_source.replace(matchPos, match.length(0), includedFileContent);

				searchStart = a_source.cbegin() + matchPos;
			}

			return;
		}

		std::string createVertexGeometryShaderSource(bool a_useRig, bool a_useShading)
		{
			std::vector<std::pair<std::string_view, std::string_view>> defines;
			if (a_useRig)
			{
				defines.emplace_back("USE_RIG", "1");
			}
			if (a_useShading)
			{
				defines.emplace_back("USE_SHADING", "1");
			}

			auto source = readFile(VOB_AOEGL_SHADER_DIR "core/geometry_vertex_shader.glsl");
			setDefines(source, defines);
			processIncludes(source);
			return source;
		}

		std::string oldCreateVertexGeometryShaderSource(bool a_useRig, bool a_useShading)
		{
			std::vector<std::pair<std::string_view, std::string_view>> defines;
			if (a_useRig)
			{
				defines.emplace_back("USE_RIG", "1");
			}
			if (a_useShading)
			{
				defines.emplace_back("USE_SHADING", "1");
			}

			auto source = readFile(VOB_AOEGL_CORE_SHADER_DIR_OLD "basic_vertex_shader.glsl");
			setDefines(source, defines);
			oldProcessIncludes(source);
			return source;
		}

		std::string createFragmentForwardShaderSource(std::string_view const& a_shadingSource)
		{
			auto source = readFile(VOB_AOEGL_CORE_SHADER_DIR_OLD "header_forward_fragment_shader.glsl");
			source.append(a_shadingSource);
			oldProcessIncludes(source);
			return source;
		}

		std::string createVertexPostProcessShaderSource()
		{
			auto source = readFile(VOB_AOEGL_CORE_SHADER_DIR_OLD "post_process_vertex_shader.glsl");
			oldProcessIncludes(source);
			return source;
		}
	}

	GraphicId oldCreateLightClusteringProgram(int32_t a_workGroupSize)
	{
		std::vector<std::pair<std::string_view, std::string_view>> defines;
		auto const workGroupSizeStr = std::to_string(a_workGroupSize);
		defines.emplace_back("WORK_GROUP_SIZE", workGroupSizeStr);

		auto computeShaderSource = readFile(VOB_AOEGL_CORE_SHADER_DIR_OLD "light_clustering_compute_shader.glsl");
		setDefines(computeShaderSource, defines);
		oldProcessIncludes(computeShaderSource);
		return createComputeProgram(computeShaderSource);
	}

	GraphicId oldCreateDepthProgram(bool a_useRig)
	{
		auto const vertexShaderSource = oldCreateVertexGeometryShaderSource(a_useRig, false /* use shading */);
		auto const fragmentShaderSource = readFile(VOB_AOEGL_CORE_SHADER_DIR_OLD "depth_fragment_shader.glsl");

		return createProgram(vertexShaderSource, fragmentShaderSource);
	}

	GraphicId oldCreateForwardProgram(std::string_view a_shadingSource, bool a_useRig, GraphicId optionalProgramId)
	{
		auto const vertexShaderSource = oldCreateVertexGeometryShaderSource(a_useRig, true /* use shading */);
		auto const fragmentShaderSource = createFragmentForwardShaderSource(a_shadingSource);

		return createProgram(vertexShaderSource, fragmentShaderSource, optionalProgramId);
	}

	GraphicId oldCreatePostProcessProgram(std::string_view a_postProcessSource, GraphicId optionalProgramId)
	{
		auto const vertexShaderSource = createVertexPostProcessShaderSource();
		return createProgram(vertexShaderSource, a_postProcessSource, optionalProgramId);
	}

	GraphicId createLightClusteringProgram(int32_t a_workGroupSize, GraphicId a_optionalProgramId)
	{
		std::vector<std::pair<std::string_view, std::string_view>> defines;
		auto const workGroupSizeStr = std::to_string(a_workGroupSize);
		defines.emplace_back("WORK_GROUP_SIZE", workGroupSizeStr);

		auto computeShaderSource = readFile(VOB_AOEGL_SHADER_DIR "core/light_clustering_shader.glsl");
		setDefines(computeShaderSource, defines);
		processIncludes(computeShaderSource);
		return createComputeProgram(computeShaderSource, a_optionalProgramId);
	}

	GraphicId createGeometryProgram(
		std::string_view a_fragmentShaderSource, bool a_useRig, bool a_useShading, bool a_useNormal, GraphicId a_optionalProgramId)
	{
		std::vector<std::pair<std::string_view, std::string_view>> defines;
		if (a_useRig)
		{
			defines.emplace_back("USE_RIG", "1");
		}
		if (a_useShading)
		{
			defines.emplace_back("USE_SHADING", "1");
		}
		if (a_useNormal)
		{
			defines.emplace_back("USE_NORMAL", "1");
		}

		auto vertexShaderSource = readFile(VOB_AOEGL_SHADER_DIR "core/geometry_vertex_shader.glsl");
		setDefines(vertexShaderSource, defines);
		processIncludes(vertexShaderSource);

		auto fragmentShaderSource = std::string{ a_fragmentShaderSource };
		processIncludes(fragmentShaderSource);
		return createProgram(vertexShaderSource, fragmentShaderSource, a_optionalProgramId);
	}

	GraphicId createShadingProgram(std::string_view a_fragmentShaderSource, bool a_useRig, GraphicId a_optionalProgramId)
	{
		return createGeometryProgram(a_fragmentShaderSource, a_useRig, true /* use shading */, false /* use normal */, a_optionalProgramId);
	}

	GraphicId createDepthProgram(bool a_useRig, GraphicId a_optionalProgramId)
	{
		auto const fragmentShaderSource = readFile(VOB_AOEGL_SHADER_DIR "core/depth_fragment_shader.glsl");
		return createGeometryProgram(fragmentShaderSource, a_useRig, false /* use shading */, true /* use normal */, a_optionalProgramId);
	}
	
	GraphicId createShadowMapProgram(bool a_useRig, GraphicId a_optionalProgramId)
	{
		auto const fragmentShaderSource = readFile(VOB_AOEGL_SHADER_DIR "core/shadow_map_fragment_shader.glsl");
		return createGeometryProgram(fragmentShaderSource, a_useRig, false /* use shading */, false /* use normal */, a_optionalProgramId);
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

	GraphicId createSsrProgram(GraphicId a_optionalProgramId)
	{
		auto const fragmentShaderSource = readFile(VOB_AOEGL_SHADER_DIR "core/ssr_fragment_shader.glsl");
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
}
