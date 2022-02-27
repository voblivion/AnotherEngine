#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/common/data/filesystem/Text.h>
#include <vob/aoe/common/render/OpenGl.h>
#include <vob/aoe/common/render/OpenGl.h>

#include <vob/aoe/core/data/ADatabase.h>

#include <vob/misc/visitor/name_value_pair.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace vob::aoe::common
{
	class VOB_AOE_API ShaderProgram
		: public type::ADynamicType
	{
	public:
		// Methods
		auto getProgramId() const
		{
			return m_programId;
		}
		void use() const
		{
			glUseProgram(m_programId);
		}
		UniformLocation getUniformLocation(char const* a_uniformName) const
		{
			return glGetUniformLocation(m_programId, a_uniformName);
		}
		void setUniform(UniformLocation a_uniformLocation, std::int32_t const a_value) const
		{
			glUniform1i(a_uniformLocation, a_value);
		}
		void setUniform(UniformLocation a_uniformLocation, std::uint32_t const a_value) const
		{
			glUniform1ui(a_uniformLocation, a_value);
		}
		void setUniform(UniformLocation a_uniformLocation, float const a_value) const
		{
			glUniform1f(a_uniformLocation, a_value);
		}
		void setUniform(UniformLocation a_uniformLocation, glm::vec1 const& a_vector) const
		{
			glUniform1f(a_uniformLocation, a_vector.x);
		}
		void setUniform(UniformLocation a_uniformLocation, glm::vec2 const& a_vector) const
		{
			glUniform2f(a_uniformLocation, a_vector.x, a_vector.y);
		}
		void setUniform(UniformLocation a_uniformLocation, glm::vec3 const& a_vector) const
		{
			glUniform3f(a_uniformLocation, a_vector.x, a_vector.y, a_vector.z);
		}
		void setUniform(UniformLocation a_uniformLocation, glm::vec4 const& a_vector) const
		{
			glUniform4f(a_uniformLocation, a_vector.x, a_vector.y, a_vector.z, a_vector.w);
		}
		void setUniform(UniformLocation a_uniformLocation, glm::mat3 const& a_matrix) const
		{
			glUniformMatrix3fv(a_uniformLocation, 1, false, glm::value_ptr(a_matrix));
		}
		void setUniform(UniformLocation a_uniformLocation, glm::mat4 const& a_matrix) const
		{
			glUniformMatrix4fv(a_uniformLocation, 1, false, glm::value_ptr(a_matrix));
		}
		bool isReady() const
		{
			return m_isReady;
		}
		void create() const;
		void destroy() const;


        template <typename VisitorType, typename Self>
		static bool accept(VisitorType& a_visitor, Self& a_this)
        {
            a_visitor.visit(misvi::nvp("Vertex Shader Source", a_this.m_vertexShaderSource));
            a_visitor.visit(misvi::nvp("Fragment Shader Source", a_this.m_fragmentShaderSource));
			return true;
		}

	private:
		// Attributes
		mutable bool m_isReady = false;
		mutable GraphicObjectId m_programId = 0;
		std::shared_ptr<common::Text const> m_vertexShaderSource;
		std::shared_ptr<common::Text const> m_fragmentShaderSource;
	};
}

