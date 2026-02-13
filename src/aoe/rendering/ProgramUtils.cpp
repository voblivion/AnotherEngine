#include <vob/aoe/rendering/ProgramUtils.h>

#include <vob/misc/std/ignorable_assert.h>

#include <optional>
#define AOEGL_DEBUG 1
#if !defined(NDEBUG) || defined(AOEGL_DEBUG)
#include <vector>
#include <iostream>
#endif


namespace vob::aoegl
{
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
#endif
			ignorable_assert(false && "Shader compilation failed.");
			glDeleteShader(shaderId);
			return k_invalidId;
		}

		return shaderId;
	}

	GraphicId createProgram(std::string_view a_vertexShaderSource, std::string_view a_fragmentShaderSource)
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

		auto const programId = glCreateProgram();
		glAttachShader(programId, vertexShaderId);
		glAttachShader(programId, fragmentShaderId);
		glLinkProgram(programId);
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
#endif
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
}
