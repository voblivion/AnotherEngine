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

#ifndef VOB_AOEGL_CORE_SHADER_DIR
#define VOB_AOEGL_CORE_SHADER_DIR "data/shaders/"
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

	GraphicId createProgram(std::string_view a_vertexShaderSource, std::string_view a_fragmentShaderSource, GraphicId optionalProgramId)
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

		auto const programId = optionalProgramId != k_invalidId ? optionalProgramId : glCreateProgram();
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

	GraphicId createComputeProgram(std::string_view a_computeShaderSource)
	{
		auto const computeShaderId = createShader(GL_COMPUTE_SHADER, a_computeShaderSource);
		if (computeShaderId == k_invalidId)
		{
			return k_invalidId;
		}

		auto const programId = glCreateProgram();
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

	void createProgram(ProgramData const& a_programData, PostProcessProgram& a_program)
	{
		createProgram(a_programData, static_cast<Program&>(a_program));
		if (a_program.id == k_invalidId)
		{
			return;
		}

		a_program.windowSize = glGetUniformLocation(a_program.id, "u_windowSize");
	}

	namespace
	{
		std::string readFile(std::string const& a_fileName)
		{
			std::ifstream file(a_fileName);
			if (!file.is_open())
			{
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
				auto const includedFileName = std::string{ VOB_AOEGL_CORE_SHADER_DIR } + match[1].str();
				auto const includedFileContent = readFile(includedFileName);

				auto const matchPos = match.position(0) + (searchStart - a_source.cbegin());
				a_source.replace(matchPos, match.length(0), includedFileContent);

				searchStart = a_source.cbegin() + matchPos;
			}

			return;
		}

		std::string createVertexShaderSource(bool a_useRig, bool a_useShading)
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

			auto source = readFile(VOB_AOEGL_CORE_SHADER_DIR "basic_vertex_shader.glsl");
			setDefines(source, defines);
			processIncludes(source);
			return source;
		}

		std::string createFragmentForwardShaderSource(std::string_view const& a_shadingSource)
		{
			auto source = readFile(VOB_AOEGL_CORE_SHADER_DIR "header_forward_fragment_shader.glsl");
			source.append(a_shadingSource);
			processIncludes(source);
			return source;
		}
	}

	GraphicId createLightClusteringProgram(int32_t a_workGroupSize)
	{
		std::vector<std::pair<std::string_view, std::string_view>> defines;
		auto const workGroupSizeStr = std::to_string(a_workGroupSize);
		defines.emplace_back("WORK_GROUP_SIZE", workGroupSizeStr);

		auto computeShaderSource = readFile(VOB_AOEGL_CORE_SHADER_DIR "light_clustering_compute_shader.glsl");
		setDefines(computeShaderSource, defines);
		processIncludes(computeShaderSource);
		return createComputeProgram(computeShaderSource);
	}

	GraphicId createDepthProgram(bool a_useRig)
	{
		auto const vertexShaderSource = createVertexShaderSource(a_useRig, false /* use shading */);
		auto const fragmentShaderSource = readFile(VOB_AOEGL_CORE_SHADER_DIR "depth_fragment_shader.glsl");

		return createProgram(vertexShaderSource, fragmentShaderSource);
	}

	GraphicId createForwardProgram(std::string_view a_shadingSource, bool a_useRig, GraphicId optionalProgramId)
	{
		auto const vertexShaderSource = createVertexShaderSource(a_useRig, true /* use shading */);
		auto const fragmentShaderSource = createFragmentForwardShaderSource(a_shadingSource);

		return createProgram(vertexShaderSource, fragmentShaderSource, optionalProgramId);
	}

	GraphicId createDebugForwardProgram(bool a_useRig)
	{
		auto const debugForwardShadingSource = readFile(VOB_AOEGL_CORE_SHADER_DIR "debug_forward_shading.glsl");
		return createForwardProgram(debugForwardShadingSource, a_useRig);
	}
}
