#include <vob/aoe/rendering/data/program_data.h>

#include <vob/misc/std/ignorable_assert.h>

#include <iostream>


namespace vob::aoegl
{
	namespace detail
	{
		std::optional<graphic_id> create_shader(
			graphic_enum a_shaderType, std::string_view a_shaderSource)
		{
			auto const shaderId = glCreateShader(a_shaderType);
			auto const shaderSourceCStr = a_shaderSource.data();
			auto const shaderSourceSize = static_cast<graphic_int>(a_shaderSource.size());

			glShaderSource(shaderId, 1, &shaderSourceCStr, &shaderSourceSize);
			glCompileShader(shaderId);
			graphic_int compilationStatus;
			glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compilationStatus);
			if (compilationStatus != GL_TRUE)
			{
#ifndef NDEBUG
				graphic_int errorLogLength;
				glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &errorLogLength);
				std::vector<char> rawErrorLog;
				rawErrorLog.resize(errorLogLength);
				glGetShaderInfoLog(shaderId, errorLogLength, &errorLogLength, rawErrorLog.data());
				std::string_view errorLog{
					rawErrorLog.data(), static_cast<std::size_t>(errorLogLength) };
				std::cerr << errorLog << std::endl;
#endif
				ignorable_assert(false && "Shader compilation failed.");
				glDeleteShader(shaderId);
				return std::nullopt;
			}

			return shaderId;
		}
	}

	void create_program(program_data const& a_programData, program& a_program)
	{
		a_program.m_id = 0;

		if (a_programData.m_vertexShaderSource == nullptr
			|| a_programData.m_fragmentShaderSource == nullptr)
		{
			ignorable_assert(false && "Missing shader source");
			return;
		}

		auto const vertexShaderId = detail::create_shader(
			GL_VERTEX_SHADER, *a_programData.m_vertexShaderSource);
		if (vertexShaderId == std::nullopt)
		{
			return;
		}

		auto const fragmentShaderId = detail::create_shader(
			GL_FRAGMENT_SHADER, *a_programData.m_fragmentShaderSource);
		if (fragmentShaderId == std::nullopt)
		{
			glDeleteShader(*vertexShaderId);
			return;
		}

		a_program.m_id = glCreateProgram();
		glAttachShader(a_program.m_id, *vertexShaderId);
		glAttachShader(a_program.m_id, *fragmentShaderId);
		glLinkProgram(a_program.m_id);
		glDeleteShader(*vertexShaderId);
		glDeleteShader(*fragmentShaderId);
		graphic_int linkStatus;
		glGetProgramiv(a_program.m_id, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE)
		{
#ifndef NDEBUG
			graphic_int errorLogLength;
			glGetProgramiv(a_program.m_id, GL_INFO_LOG_LENGTH, &errorLogLength);
			std::vector<char> rawErrorLog;
			rawErrorLog.resize(errorLogLength);
			glGetProgramInfoLog(
				a_program.m_id, errorLogLength, &errorLogLength, rawErrorLog.data());
			std::string_view errorLog{
				rawErrorLog.data(), static_cast<std::size_t>(errorLogLength) };
			std::cerr << errorLog << std::endl;
#endif
			a_program.m_id = 0;
		}
		glUseProgram(a_program.m_id);
	}

	void create_program(program_data const& a_programData, scene_program& a_program)
	{
		create_program(a_programData, static_cast<program&>(a_program));
		if (a_program.m_id == 0)
		{
			return;
		}

		a_program.m_viewPositionLocation = glGetUniformLocation(a_program.m_id, k_viewPosition);
		a_program.m_viewProjectionTransformLocation = glGetUniformLocation(
			a_program.m_id, k_viewProjectionTransform);
	}

	void create_program(program_data const& a_programData, mesh_program& a_program)
	{
		create_program(a_programData, static_cast<scene_program&>(a_program));
		if (a_program.m_id == 0)
		{
			return;
		}

		a_program.m_ambientColorLocation = glGetUniformLocation(a_program.m_id, k_ambientColor);
		a_program.m_sunColorLocation = glGetUniformLocation(a_program.m_id, k_sunColor);
		a_program.m_sunDirectionLocation = glGetUniformLocation(a_program.m_id, k_sunDirection);
		a_program.m_meshTransformLocation = glGetUniformLocation(a_program.m_id, k_meshTransform);
		a_program.m_meshNormalTransformLocation = glGetUniformLocation(
			a_program.m_id, k_meshNormalTransform);
		a_program.m_isRiggedLocation = glGetUniformLocation(a_program.m_id, k_isRigged);
		a_program.m_rigPoseLocation = glGetUniformLocation(a_program.m_id, k_rigPose);
	}

	void create_program(program_data const& a_programData, post_process_program& a_program)
	{
		create_program(a_programData, static_cast<program&>(a_program));
		if (a_program.m_id == 0)
		{
			return;
		}

		a_program.m_windowSize = glGetUniformLocation(a_program.m_id, k_windowSize);
	}
}
