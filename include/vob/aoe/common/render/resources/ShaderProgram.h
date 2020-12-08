#pragma once

#include <vob/aoe/common/render/OpenGl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vob/aoe/api.h>

#include <vob/aoe/core/data/ADatabase.h>
#include <vob/aoe/core/data/Handle.h>
#include <vob/aoe/core/visitor/Utils.h>

#include <vob/aoe/common/data/filesystem/Text.h>
#include <vob/aoe/common/render/OpenGl.h>
#include <vob/aoe/core/type/Primitive.h>

namespace vob::aoe::common
{
	class VOB_AOE_API ShaderProgram
		: public type::ADynamicType
	{
	public:
		// Constructors / Destructor
		explicit ShaderProgram(data::ADatabase& a_database);

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
		void setUniform(UniformLocation a_uniformLocation, u32 const a_value) const
		{
			glUniform1ui(a_uniformLocation, a_value);
		}
		void setUniform(UniformLocation a_uniformLocation, float const a_value) const
		{
			glUniform1f(a_uniformLocation, a_value);
		}
		void setUniform(UniformLocation a_uniformLocation, vec1 const& a_vector) const
		{
			glUniform1f(a_uniformLocation, a_vector.x);
		}
		void setUniform(UniformLocation a_uniformLocation, vec2 const& a_vector) const
		{
			glUniform2f(a_uniformLocation, a_vector.x, a_vector.y);
		}
		void setUniform(UniformLocation a_uniformLocation, vec3 const& a_vector) const
		{
			glUniform3f(a_uniformLocation, a_vector.x, a_vector.y, a_vector.z);
		}
		void setUniform(UniformLocation a_uniformLocation, vec4 const& a_vector) const
		{
			glUniform4f(a_uniformLocation, a_vector.x, a_vector.y, a_vector.z, a_vector.w);
		}
		void setUniform(UniformLocation a_uniformLocation, mat3 const& a_matrix) const
		{
			glUniformMatrix3fv(a_uniformLocation, 1, false, glm::value_ptr(a_matrix));
		}
		void setUniform(UniformLocation a_uniformLocation, mat4 const& a_matrix) const
		{
			glUniformMatrix4fv(a_uniformLocation, 1, false, glm::value_ptr(a_matrix));
		}
		bool isReady() const
		{
			return m_isReady;
		}
		void create() const;
		void destroy() const;

	public: // TODO -> how to make accept friend ?
		// Attributes
		mutable bool m_isReady = false;
		mutable GraphicObjectId m_programId = 0;
		data::Handle<common::Text> m_vertexShaderSource;
		data::Handle<common::Text> m_fragmentShaderSource;
	};
}

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ThisType>
	visitIfType<common::ShaderProgram, ThisType> accept(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(vis::makeNameValuePair("Vertex Shader Source", a_this.m_vertexShaderSource));
		a_visitor.visit(vis::makeNameValuePair("Fragment Shader Source", a_this.m_fragmentShaderSource));
	}
}
