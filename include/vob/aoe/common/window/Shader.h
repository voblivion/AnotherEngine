#pragma once

#include <string>
#include <GL/glew.h>
#include <vob/aoe/core/standard/ADynamicType.h>

namespace vob::aoe::common
{
	class Shader
		: public ADynamicType
	{
	public:
		// Aliases
		enum class Type
		{
			Vertex,
			Geometry,
			Fragment
		};

		// Constructors
		Shader(Type const a_type, std::string_view const a_source)
		{
			switch (a_type)
			{
			case Type::Vertex:
				m_openglId = glCreateShader(GL_VERTEX_SHADER);
				break;
			case Type::Geometry:
				m_openglId = glCreateShader(GL_GEOMETRY_SHADER);
				break;
			case Type::Fragment:
				m_openglId = glCreateShader(GL_FRAGMENT_SHADER);
				break;
			}

			auto t_sourceStr = a_source.data();
			auto t_sourceSize = static_cast<std::int32_t>(a_source.size());
			glShaderSource(m_openglId, 1, &t_sourceStr, &t_sourceSize);
			glCompileShader(m_openglId);

			// TODO remove
			std::int32_t t_success;
			glGetShaderiv(m_openglId, GL_COMPILE_STATUS, &t_success);
			if (!t_success)
			{
				char infoLog[512];
				int s;
				glGetShaderInfoLog(m_openglId, 512, &s, infoLog);
				std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
			}
		}

		Shader(Shader&&) = delete; // TODO
		Shader(Shader const&) = delete;

		~Shader()
		{
			glDeleteShader(m_openglId);
		}

		// Methods
		bool isValid() const
		{
			std::int32_t t_success;
			glGetShaderiv(m_openglId, GL_COMPILE_STATUS, &t_success);
			return t_success == GL_TRUE;
		}

		std::uint32_t getOpenglId() const
		{
			return m_openglId;
		}

		// Operators
		Shader& operator=(Shader&&) = delete; // TODO
		Shader& operator=(Shader const&) = delete;

	private:
		// Attributes
		std::uint32_t m_openglId = 0;
	};

	template <Shader::Type type>
	class SpecialShader final
		: public Shader
	{
	public:
		// Constructors
		explicit SpecialShader(std::string_view const a_source)
			: Shader{ type, a_source }
		{}
	};

	using VertexShader = SpecialShader<Shader::Type::Vertex>;
	using GeometryShader = SpecialShader<Shader::Type::Geometry>;
	using FragmentShader = SpecialShader<Shader::Type::Fragment>;
}
