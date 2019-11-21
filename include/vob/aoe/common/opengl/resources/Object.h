#pragma once

#include <cassert>
#include <array>
#include <cstdint>
#include <GL/glew.h>

namespace vob::aoe::ogl
{
#define VOB_AOE_OGL_DECLARE_OBJECT(Name) \
	struct Name##Object \
	{ \
		void create() const; \
		bool isReady() const; \
		void destroy() const; \
		mutable std::uint32_t m_id = 0; \
	}

	VOB_AOE_OGL_DECLARE_OBJECT(Buffer);
	VOB_AOE_OGL_DECLARE_OBJECT(Framebuffer);
	VOB_AOE_OGL_DECLARE_OBJECT(Program);
	VOB_AOE_OGL_DECLARE_OBJECT(Renderbuffer);
	VOB_AOE_OGL_DECLARE_OBJECT(Texture);
	VOB_AOE_OGL_DECLARE_OBJECT(VertexArray);
#undef VOB_AOE_OGL_DECLARE_OBJECT

	enum class ShaderType
	{
		Fragment
		, Vertex
	};

	constexpr auto toGLenum(ShaderType a_shaderType)
	{
		constexpr std::array<GLenum, 2> s_values = {
			GL_FRAGMENT_SHADER
			, GL_VERTEX_SHADER
		};

		return s_values[static_cast<std::underlying_type_t<ShaderType>>(a_shaderType)];
	}

	template <ShaderType t_shaderType>
	struct ShaderObject
	{
		void create() const
		{
			assert(!isReady());
			m_id = glCreateShader(toGLenum(t_shaderType));
		}

		bool isReady() const
		{
			return m_id != 0;
		}

		void destroy() const
		{
			assert(isReady());
			glDeleteShader(m_id);
			m_id = 0;
		}

		mutable std::uint32_t m_id = 0;
	};

	using FragmentShaderObject = ShaderObject<ShaderType::Fragment>;
	using VertexShaderObject = ShaderObject<ShaderType::Vertex>;
}

// TODO : put that in .cpp

namespace vob::aoe::ogl
{
#define VOB_AOE_OGL_DEFINE_OBJECT_GEN_DELETE(Name) \
	void Name##Object::create() const \
	{ \
		assert(!isReady()); \
		glGen##Name##s(1, &m_id); \
	} \
	bool Name##Object::isReady() const \
	{ \
		return m_id != 0; \
	} \
	void Name##Object::destroy() const \
	{ \
		assert(isReady()); \
		glDelete##Name##s(1, &m_id); \
		m_id = 0; \
	}

	VOB_AOE_OGL_DEFINE_OBJECT_GEN_DELETE(Buffer);
	VOB_AOE_OGL_DEFINE_OBJECT_GEN_DELETE(Framebuffer);
	VOB_AOE_OGL_DEFINE_OBJECT_GEN_DELETE(Renderbuffer);
	VOB_AOE_OGL_DEFINE_OBJECT_GEN_DELETE(Texture);
	VOB_AOE_OGL_DEFINE_OBJECT_GEN_DELETE(VertexArray);

#undef VOB_AOE_OGL_DEFINE_OBJECT_GEN_DELETE

	inline void ProgramObject::create() const
	{
		assert(!isReady());
		m_id = glCreateProgram();
	}

	inline bool ProgramObject::isReady() const
	{
		return m_id != 0;
	}


	inline void ProgramObject::destroy() const
	{
		assert(isReady());
		glDeleteProgram(m_id);
		m_id = 0;
	}
}
