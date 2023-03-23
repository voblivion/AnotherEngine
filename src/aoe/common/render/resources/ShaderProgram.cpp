#include <vob/aoe/common/_render/resources/ShaderProgram.h>

#include <iostream>

#include <vob/misc/std/ignorable_assert.h>


namespace vob::aoe::common
{
	enum class ObjectType
	{
		Shader
		, Program
	};

	template <ObjectType>
	struct Get;

	template <>
	struct Get<ObjectType::Shader>
	{
		static constexpr GLenum c_isSuccessValueName = GL_COMPILE_STATUS;

		static void integerValue(GraphicObjectId a_shaderId, GLenum a_valueName, GLint* a_value)
		{
			glGetShaderiv(a_shaderId, a_valueName, a_value);
		}

		static void infoLog(
			GraphicObjectId a_shaderId
			, GLint a_availableLength
			, GLint* a_outLength
			, char* a_infoLog
		)
		{
			glGetShaderInfoLog(a_shaderId, a_availableLength, a_outLength, a_infoLog);
		}
	};

	template <>
	struct Get<ObjectType::Program>
	{
		static constexpr GLenum c_isSuccessValueName = GL_LINK_STATUS;

		static void integerValue(GraphicObjectId a_programId, GLenum a_valueName, GLint* a_value)
		{
			glGetProgramiv(a_programId, a_valueName, a_value);
		}

		static void infoLog(
			GraphicObjectId a_programId
			, GLint a_availableLength
			, GLint* a_outLength
			, char* a_infoLog
		)
		{
			glGetProgramInfoLog(a_programId, a_availableLength, a_outLength, a_infoLog);
		}
	};

	void getProgramIntegerValue(GraphicObjectId a_programId, GLenum a_valueName, GLint* a_value)
	{
		glGetProgramiv(a_programId, a_valueName, a_value);
	}

	template <ObjectType t_objectType>
	auto getIntegerValue(GraphicObjectId a_programId, GLenum a_valueName)
	{
		GLint value;
		Get<t_objectType>::integerValue(a_programId, a_valueName, &value);
		return value;
	}

	template <ObjectType t_objectType>
	std::string getInfoLog(GraphicObjectId a_objectId)
	{
		auto length = getIntegerValue<t_objectType>(a_objectId, GL_INFO_LOG_LENGTH);
		std::vector<char> rawInfoLog;
		rawInfoLog.resize(length);
		Get<t_objectType>::infoLog(a_objectId, length, &length, rawInfoLog.data());
		return { rawInfoLog.data(), static_cast<std::size_t>(length) };
	}

	template <ObjectType t_objectType>
	bool hasSucceeded(GraphicObjectId a_objectId)
	{
		return getIntegerValue<t_objectType>(a_objectId, Get<t_objectType>::c_isSuccessValueName) == GL_TRUE;
		// TODO : better error handling
	}

	std::optional<GraphicObjectId> createShader(GLenum a_shaderType, std::string_view a_shaderSource)
	{
		auto const shaderId = glCreateShader(a_shaderType);

		auto shaderSourceCStr = a_shaderSource.data();
		auto shaderSourceSize = static_cast<GLint>(a_shaderSource.size());

		glShaderSource(shaderId, 1, &shaderSourceCStr, &shaderSourceSize);
		glCompileShader(shaderId);
		if (!hasSucceeded<ObjectType::Shader>(shaderId))
		{
			ignorable_assert(false);
#ifndef NDEBUG
			auto const infoLog = getInfoLog<ObjectType::Shader>(shaderId);
			std::cerr << infoLog << std::endl;
#endif
			return std::nullopt;
		}

		return shaderId;
	}

	void ShaderProgram::create() const
	{
		assert(!isReady());
		if (m_vertexShaderSource == nullptr || m_fragmentShaderSource == nullptr)
		{
			return;
		}

		auto const vertexShaderId = createShader(GL_VERTEX_SHADER, m_vertexShaderSource->m_string);
		if (!vertexShaderId.has_value())
		{
			return;
		}

		auto const fragmentShaderId = createShader(GL_FRAGMENT_SHADER, m_fragmentShaderSource->m_string);
		if (!fragmentShaderId.has_value())
		{
			glDeleteShader(vertexShaderId.value());
			return;
		}

		m_programId = glCreateProgram();
		glAttachShader(m_programId, vertexShaderId.value());
		glAttachShader(m_programId, fragmentShaderId.value());
		glLinkProgram(m_programId);
		glDeleteShader(vertexShaderId.value());
		glDeleteShader(fragmentShaderId.value());
		if (!hasSucceeded<ObjectType::Program>(m_programId))
		{
			ignorable_assert(false);
#ifndef NDEBUG
			auto const infoLog = getInfoLog<ObjectType::Program>(m_programId);
			std::cerr << infoLog << std::endl;
#endif
			glDeleteProgram(m_programId);
			return;
		}

		m_isReady = true;
	}

	void ShaderProgram::destroy() const
	{
		if (m_isReady)
		{
			m_isReady = false;
			glDeleteProgram(m_programId);
		}
	}

}
