#pragma once

#include <vob/aoe/rendering/resources/program.h>

#include <vob/misc/visitor/macros.h>

#include <string>
#include <memory>


namespace vob::aoegl
{
	constexpr char const* k_viewPosition = "u_viewPosition";
	constexpr char const* k_viewProjectionTransform = "u_viewProjectionTransform";
	constexpr char const* k_ambientColor = "u_ambientColor";
	constexpr char const* k_sunColor = "u_sunColor";
	constexpr char const* k_sunDirection = "u_sunDirection";
	constexpr char const* k_meshTransform = "u_meshTransform";
	constexpr char const* k_meshNormalTransform = "u_meshNormalTransform";
	constexpr char const* k_isRigged = "u_isRigged";
	constexpr char const* k_rigPose = "u_rigPose";
	constexpr char const* k_windowSize = "u_windowSize";

	struct program_data
	{
		std::shared_ptr<std::pmr::string const> m_vertexShaderSource;
		std::shared_ptr<std::pmr::string const> m_fragmentShaderSource;
	};

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
// component manager
// physic material
namespace vob::misvi
{
	VOB_MISVI_ACCEPT(aoegl::program_data)
	{
		VOB_MISVI_NVP("Vertex Shader", vertexShaderSource);
		VOB_MISVI_NVP("Fragment Shader", fragmentShaderSource);
		return true;
	}
}
