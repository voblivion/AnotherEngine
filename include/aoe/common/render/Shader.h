#pragma once

#include <array>
#include <iostream>
#include <string>

#include <GL/glew.h>

#include <aoe/core/standard/ADynamicType.h>
#include <aoe/core/standard/IgnorableAssert.h>

#include <aoe/common/render/GlObjects.h>

namespace aoe
{
	namespace common
	{
		template <GLenum shaderType>
		struct Shader final
			: public sta::ADynamicType
		{
			void loadFrom(std::string_view const a_source)
			{
				m_glShader.tryCreate();
				auto t_sourceCStr = a_source.data();
				auto t_sourceSize = static_cast<std::int32_t>(a_source.size());
				glShaderSource(m_glShader.m_id, 1, &t_sourceCStr, &t_sourceSize);
				glCompileShader(m_glShader.m_id);
				{
					std::int32_t t_success;
					glGetShaderiv(m_glShader.m_id, GL_COMPILE_STATUS, &t_success);
					ignorableAssert(t_success != 0);
					if (!t_success)
					{
						std::array<char, 512> t_errorLog{};
						glGetShaderInfoLog(m_glShader.m_id, t_errorLog.size(), nullptr
							, t_errorLog.data());
						std::cerr << t_errorLog.data() << std::endl;
						m_glShader.release();
					}
				}
			}

			gl::Shader<shaderType> m_glShader;
		};

		using VertexShader = Shader<GL_VERTEX_SHADER>;
		using GeometryShader = Shader<GL_GEOMETRY_SHADER>;
		using FragmentShader = Shader<GL_FRAGMENT_SHADER>;
	}
}
