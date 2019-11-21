#pragma once

#include <array>
#include <iostream>
#include <string>

#include <GL/glew.h>

#include <vob/sta/ignorable_assert.h>

#include <vob/aoe/common/opengl/resources/Object.h>

namespace vob::aoe::ogl
{
	template <ShaderType t_shaderType>
	struct ShaderResource
	{
		explicit ShaderResource(std::pmr::string const& a_source)
			: m_source{ a_source }
		{}

		void create() const
		{
			m_object.create();
			auto sourceCStr = m_source.data();
			auto sourceSize = static_cast<GLint>(m_source.size());
			glShaderSource(m_object.m_id, 1, &sourceCStr, &sourceSize);
			glCompileShader(m_object.m_id);

			// TODO : error management ?
			{
				std::int32_t t_success;
				glGetShaderiv(m_object.m_id, GL_COMPILE_STATUS, &t_success);
				ignorable_assert(t_success == GL_TRUE);
				if (t_success != GL_TRUE)
				{
					std::array<char, 512> t_errorLog{};
					glGetShaderInfoLog(m_object.m_id
						, static_cast<GLsizei>(t_errorLog.size()), nullptr
						, t_errorLog.data());
					std::cerr << t_errorLog.data() << std::endl;
					m_object.destroy();
				}
			}
		}

		void destroy() const
		{
			m_object.destroy();
		}

		std::pmr::string const& m_source;
		ShaderObject<t_shaderType> m_object;
	};

	using FragmentShaderResource = ShaderResource<ShaderType::Fragment>;
	using VertexShaderResource = ShaderResource<ShaderType::Vertex>;
}
