#pragma once

#include <cassert>
#include <cstdint>

#include <GL/glew.h>

namespace aoe::gl
{
#define __GL_DEFINE_GEN_RESOURCE_FUNCTION(Name) \
	inline auto create##Name() { std::uint32_t r_id; glGen##Name##s(1, &r_id); return r_id; }

#define __GL_DEFINE_DELETE_RESOURCE_FUNCTION(Name) \
	inline auto release##Name(std::uint32_t a_id) { glDelete##Name##s(1, &a_id); } \

#define __GL_DEFINE_BIND_RESOURCE_FUNCTION(Name) \
	inline auto use##Name(std::uint32_t a_id) { glBind##Name(a_id); }

#define __GL_DEFINE_GEN_DELETE_RESOURCE_FUNCTIONS(Name) \
	__GL_DEFINE_GEN_RESOURCE_FUNCTION(Name) \
	__GL_DEFINE_DELETE_RESOURCE_FUNCTION(Name)


#define __GL_DEFINE_GEN_DELETE_BIND_RESOURCE_FUNCTIONS(Name) \
	__GL_DEFINE_GEN_DELETE_RESOURCE_FUNCTIONS(Name) \
	__GL_DEFINE_BIND_RESOURCE_FUNCTION(Name)

#define __GL_GET_RESOURCE_FUNCTIONS(Name) \
	create##Name, release##Name, use##Name

	// GlResource
	template <std::uint32_t createFn(), void releaseFn(std::uint32_t)
		, void useFn(std::uint32_t), std::uint32_t invalidId = 0>
		struct  Resource
	{
		// Constructors
		Resource() = default;
		Resource(Resource&& a_other) noexcept
		{
			m_id = a_other.m_id;
			a_other.m_id = invalidId;
		}
		Resource(Resource const&) = delete;

		~Resource()
		{
			tryRelease();
		}

		// Operators
		Resource& operator=(Resource&& a_other) noexcept
		{
			tryRelease();
			m_id = a_other.m_id;
			a_other.m_id = invalidId;
			return *this;
		}
		Resource& operator=(Resource const&) = delete;

		// Methods
		void create()
		{
			assert(!isValid());
			m_id = createFn();
		}

		void tryCreate()
		{
			if (!isValid())
			{
				create();
			}
		}

		void release()
		{
			assert(isValid());
			releaseFn(m_id);
			m_id = invalidId;
		}

		void tryRelease()
		{
			if (isValid())
			{
				release();
			}
		}

		void reset()
		{
			tryRelease();
			create();
		}

		bool isValid() const
		{
			return m_id != invalidId;
		}

		void startUsing() const
		{
			assert(isValid());
			useFn(m_id);
		}

		void stopUsing() const
		{
			useFn(0);
		}

		// Attributes
		std::uint32_t m_id = invalidId;
	};

	template <typename Resource>
	struct ResourceScopeUse
	{
		explicit ResourceScopeUse(Resource const& a_resource)
			: m_resource{ a_resource }
		{
			m_resource.startUsing();
		}

		ResourceScopeUse(ResourceScopeUse const&) = delete;
		ResourceScopeUse(ResourceScopeUse&&) = default;

		ResourceScopeUse& operator=(ResourceScopeUse const&) = delete;
		ResourceScopeUse& operator=(ResourceScopeUse&&) = default;

		~ResourceScopeUse()
		{
			m_resource.stopUsing();
		}

		Resource const& m_resource;
	};

#define __GL_DEFINE_RESOURCE(Name) \
	using Name = Resource<__GL_GET_RESOURCE_FUNCTIONS(Name)>

	// Texture
	__GL_DEFINE_GEN_DELETE_RESOURCE_FUNCTIONS(Texture);
	inline auto useTexture(std::uint32_t t_id) { glBindTexture(GL_TEXTURE_2D, t_id); };
	__GL_DEFINE_RESOURCE(Texture);

	// Renderbuffer
	__GL_DEFINE_GEN_DELETE_RESOURCE_FUNCTIONS(Renderbuffer);
	inline auto useRenderbuffer(std::uint32_t a_id) { glBindRenderbuffer(GL_RENDERBUFFER, a_id); }
	__GL_DEFINE_RESOURCE(Renderbuffer);

	// Framebuffer
	__GL_DEFINE_GEN_DELETE_RESOURCE_FUNCTIONS(Framebuffer);
	inline auto useFramebuffer(std::uint32_t a_id) { glBindFramebuffer(GL_FRAMEBUFFER, a_id); }
	__GL_DEFINE_RESOURCE(Framebuffer);

	// Program
	inline auto createProgram() { return static_cast<std::uint32_t>(glCreateProgram()); }
	inline auto releaseProgram(std::uint32_t a_id) { glDeleteProgram(a_id); }
	inline auto useProgram(std::uint32_t a_id) { glUseProgram(a_id); }
	__GL_DEFINE_RESOURCE(Program);

	// Shader
	template <GLenum shaderType>
	struct CreateShaderFunctor
	{
		static std::uint32_t create()
		{
			return static_cast<std::uint32_t>(glCreateShader(shaderType));
		}
	};
	inline auto releaseShader(std::uint32_t a_id) { glDeleteShader(a_id); }
	inline auto useShader(std::uint32_t a_id) {}

	template <GLenum shaderType>
	using Shader = Resource<CreateShaderFunctor<shaderType>::create, releaseShader, useShader>;

	// VertexArray
	__GL_DEFINE_GEN_DELETE_BIND_RESOURCE_FUNCTIONS(VertexArray)
		__GL_DEFINE_RESOURCE(VertexArray);

	// Buffer
	__GL_DEFINE_GEN_DELETE_RESOURCE_FUNCTIONS(Buffer)
		inline auto useBuffer(std::uint32_t a_id) { glBindBuffer(GL_ARRAY_BUFFER, a_id); }
	__GL_DEFINE_RESOURCE(Buffer);
}