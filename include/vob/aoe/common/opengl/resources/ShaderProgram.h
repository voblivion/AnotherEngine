#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vob/aoe/core/type/ADynamicType.h>
#include <vob/aoe/core/data/ADatabase.h>
#include <vob/aoe/core/data/Handle.h>
#include <vob/aoe/core/visitor/Utils.h>
#include "vob/aoe/core/visitor/Aggregate.h"

#include <vob/aoe/common/opengl/Guard.h>
#include <vob/aoe/common/render/Shader.h>
#include <vob/aoe/common/data/Text.h>

namespace vob::aoe::ogl
{
	class ShaderProgram
		: public vis::Aggregate<ShaderProgram>
	{
	public:
		explicit ShaderProgram(data::ADatabase& a_database)
			: m_vertexShaderSource{ a_database }
			, m_fragmentShaderSource{ a_database }
		{}

		void create() const
		{
			if (!m_vertexShaderSource.isValid() || !m_fragmentShaderSource.isValid())
			{
				// TODO : error ?
				return;
			}

			Guard<VertexShaderResource> vertexShader{ m_vertexShaderSource->m_string };

			Guard<FragmentShaderResource> fragmentShader{ m_fragmentShaderSource->m_string };

			m_object.create();

			glAttachShader(m_object.m_id, vertexShader.resource().m_object.m_id);
			glAttachShader(m_object.m_id, fragmentShader.resource().m_object.m_id);
			glLinkProgram(m_object.m_id);

			// TODO : error ?
			{
				std::int32_t t_success = 0;
				glGetProgramiv(m_object.m_id, GL_LINK_STATUS, &t_success);
				ignorable_assert(t_success == GL_TRUE);
				if (t_success != GL_TRUE)
				{
					std::array<char, 512> t_errorLog{};
					glGetProgramInfoLog(
						m_object.m_id
						, static_cast<GLsizei>(t_errorLog.size())
						, nullptr
						, t_errorLog.data()
					);
					std::cerr << t_errorLog.data() << std::endl;
				}
			}
		}

		bool isReady() const
		{
			return m_object.isReady();
		}

		void destroy() const
		{
			assert(!isReady());
			m_object.destroy();
		}

		std::int32_t getUniformLocation(std::string_view a_name) const
		{
			return glGetUniformLocation(m_object.m_id, a_name.data());
		}

		void setUniform(std::string_view const a_name, bool const a_value) const
		{
			glUniform1i(getUniformLocation(a_name), static_cast<std::int32_t>(a_value));
		}

		void setUniform(std::string_view const a_name, std::int32_t const a_value) const
		{
			glUniform1i(getUniformLocation(a_name), a_value);
		}

		void setUniform(std::string_view const a_name, float const a_value) const
		{
			glUniform1f(getUniformLocation(a_name), a_value);
		}

		void setUniform(std::string_view const a_name, glm::vec3 const a_value) const
		{
			glUniform3f(getUniformLocation(a_name), a_value.x, a_value.y, a_value.z);
		}

		void setUniform(std::string_view const a_name, glm::vec4 const a_value) const
		{
			glUniform4f(getUniformLocation(a_name), a_value.x, a_value.y, a_value.z, a_value.w);
		}

		void setUniform(std::string_view const a_name, glm::mat3 const a_value) const
		{
			glUniformMatrix3fv(getUniformLocation(a_name), 1, GL_FALSE, glm::value_ptr(a_value));
		}

		void setUniform(std::string_view const a_name, glm::mat4 const a_value) const
		{
			glUniformMatrix4fv(getUniformLocation(a_name), 1, GL_FALSE, glm::value_ptr(a_value));
		}

		void use() const
		{
			glUseProgram(m_object.m_id);
		}
		
	private:
		ProgramObject m_object;
		data::Handle<common::Text> m_vertexShaderSource;
		data::Handle<common::Text> m_fragmentShaderSource;

		friend class vis::Aggregate<ShaderProgram>;
		template <typename VisitorType, typename ThisType>
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
			a_visitor.visit(vis::makeNameValuePair("Vertex Shader Source", a_this.m_vertexShaderSource));
			a_visitor.visit(vis::makeNameValuePair("Fragment Shader Source", a_this.m_fragmentShaderSource));
		}
	};
}
