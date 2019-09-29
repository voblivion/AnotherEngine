#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <aoe/core/standard/ADynamicType.h>
#include <aoe/core/data/ADatabase.h>
#include <aoe/core/data/Handle.h>
#include <aoe/core/visitor/Utils.h>
#include "aoe/core/visitor/Aggregate.h"

#include <aoe/common/render/Shader.h>

namespace aoe
{
	namespace common
	{
		class ShaderProgram final
			: public vis::Aggregate<ShaderProgram, sta::ADynamicType>
		{
		public:
			// Constructors
			explicit ShaderProgram(data::ADatabase& a_database)
				: m_vertexShader{ a_database }
				, m_fragmentShader{ a_database }
			{}

			// Methods
			void compile()
			{
				if(!m_vertexShader.isValid()
					&& m_vertexShader->m_glShader.isValid())
				{
					return;
				}
				if(!m_fragmentShader.isValid()
					&& m_fragmentShader->m_glShader.isValid())
				{
					return;
				}
				m_glProgram.tryCreate();

				glAttachShader(m_glProgram.m_id, m_vertexShader->m_glShader.m_id);
				glAttachShader(m_glProgram.m_id, m_fragmentShader->m_glShader.m_id);
				glLinkProgram(m_glProgram.m_id);
				{
					std::int32_t t_success;
					glGetProgramiv(m_glProgram.m_id, GL_COMPILE_STATUS, &t_success);
					ignorableAssert(t_success != 0);
					if (!t_success)
					{
						std::array<char, 512> t_errorLog{};
						glGetProgramInfoLog(m_glProgram.m_id, t_errorLog.size(), nullptr
							, t_errorLog.data());
						std::cerr << t_errorLog.data() << std::endl;
					}
				}
			}

			std::int32_t getUniformLocation(std::string_view a_name) const
			{
				return glGetUniformLocation(m_glProgram.m_id, a_name.data());
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

			template <typename VisitorType>
			void accept(VisitorType& a_visitor)
			{
				a_visitor.visit(vis::makeNameValuePair("Vertex Shader"
					, m_vertexShader));
				a_visitor.visit(vis::makeNameValuePair("Fragment Shader"
					, m_fragmentShader));
				compile();
			}

			// Attributes
			gl::Program m_glProgram;
			data::Handle<VertexShader> m_vertexShader;
			// data::Handle<Shader> m_geometryShader;
			data::Handle<FragmentShader> m_fragmentShader;

		private:
			// Methods
			void postAccept()
			{
				compile();
			}

			friend class vis::Aggregate<ShaderProgram, sta::ADynamicType>;
			template <typename VisitorType, typename ThisType>
			static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
			{
				a_visitor.visit(vis::makeNameValuePair("Vertex Shader"
					, a_this.m_vertexShader));
				a_visitor.visit(vis::makeNameValuePair("Fragment Shader"
					, a_this.m_fragmentShader));
			}
		};
	}
}
