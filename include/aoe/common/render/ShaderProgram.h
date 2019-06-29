#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <aoe/core/standard/ADynamicType.h>
#include <aoe/core/data/ADatabase.h>
#include <aoe/core/data/Handle.h>
#include <aoe/common/render/Shader.h>

namespace aoe
{
	namespace common
	{
		class ShaderProgram final
			: public sta::ADynamicType
		{
		public:
			explicit ShaderProgram(data::ADatabase& a_database)
				: m_vertexShader{ a_database }
				, m_fragmentShader{ a_database }
			{
				m_openglId = glCreateProgram();
			}
			ShaderProgram(ShaderProgram&&) = delete; // TODO
			ShaderProgram(ShaderProgram const&) = delete;

			~ShaderProgram()
			{
				glDeleteProgram(m_openglId);
			}

			template <typename VisitorType>
			void accept(VisitorType& a_visitor)
			{
				a_visitor.visit("Vertex Shader", m_vertexShader);
				a_visitor.visit("Fragment Shader", m_fragmentShader);
				m_isDirty = true;
			}

			// Methods
			bool isValid() const
			{
				if (m_isDirty)
				{
					compile();
				}
				std::int32_t t_success;
				glGetProgramiv(m_openglId, GL_LINK_STATUS, &t_success);

				// TODO remove
				if (!t_success)
				{
					char infoLog[512];
					int s;
					glGetProgramInfoLog(m_openglId, 512, &s, infoLog);
					std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
				}
				return t_success == GL_TRUE;
			}

			void use() const
			{
				if (m_isDirty)
				{
					compile();
				}
				glUseProgram(m_openglId);
			}

			void setUniform(std::string_view const a_name, bool const a_value) const
			{
				glUniform1i(getUniformLocation(a_name)
					, static_cast<std::int32_t>(a_value));
			}

			void setUniform(std::string_view const a_name
				, std::int32_t const a_value) const
			{
				glUniform1i(getUniformLocation(a_name), a_value);
			}

			void setUniform(std::string_view const a_name
				, float const a_value) const
			{
				glUniform1f(getUniformLocation(a_name), a_value);
			}

			void setUniform(std::string_view const a_name
				, glm::vec3 const a_value) const
			{
				glUniform3f(getUniformLocation(a_name)
					, a_value.x, a_value.y, a_value.z);
			}

			void setUniform(std::string_view const a_name
				, glm::vec4 const a_value) const
			{
				glUniform4f(getUniformLocation(a_name)
					, a_value.x, a_value.y, a_value.z, a_value.w);
			}

			void setUniform(std::string_view const a_name
				, glm::mat3 const a_value) const
			{
				glUniformMatrix3fv(getUniformLocation(a_name)
					, 1, GL_FALSE, glm::value_ptr(a_value));
			}

			void setUniform(std::string_view const a_name
				, glm::mat4 const a_value) const
			{
				glUniformMatrix4fv(getUniformLocation(a_name)
					, 1, GL_FALSE, glm::value_ptr(a_value));
			}

			// Operators
			ShaderProgram& operator=(ShaderProgram&&) = delete; // TODO
			ShaderProgram& operator=(ShaderProgram const&) = delete;

		private:
			// Attributes
			std::uint32_t m_openglId;
			mutable bool m_isDirty = false;
			data::Handle<VertexShader> m_vertexShader;
			// DataHandle<Shader> m_geometryShader;
			data::Handle<FragmentShader> m_fragmentShader;

			// Methods
			void compile() const
			{
				if (m_vertexShader.isValid() && m_vertexShader->isValid())
				{
					glAttachShader(m_openglId, m_vertexShader->getOpenglId());
				}
				if (m_fragmentShader.isValid() && m_fragmentShader->isValid())
				{
					glAttachShader(m_openglId, m_fragmentShader->getOpenglId());
				}
				glLinkProgram(m_openglId);
				m_vertexShader.tryUnload();
				m_fragmentShader.tryUnload();
				m_isDirty = false;
			}

			std::int32_t getUniformLocation(std::string_view a_name) const
			{
				return glGetUniformLocation(m_openglId, a_name.data());
			}
		};
	}
}