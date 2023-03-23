#pragma once

#include "image_view.h"
#include "resource.h"
#include "shader_program.h"

#include <optional>
#include <span>
#include <string_view>
#ifndef NDEBUG
#include <iostream>
#include <vector>
#endif


namespace vob::aoegl::resource_util
{
	namespace detail
	{
		std::optional<graphic_id> create_shader(
			graphic_enum a_shaderType, std::string_view a_shaderSource)
		{
			auto const shaderId = glCreateShader(a_shaderType);
			auto const shaderSourceCStr = a_shaderSource.data();
			auto const shaderSourceSize = static_cast<graphic_int>(a_shaderSource.size());

			glShaderSource(shaderId, 1, &shaderSourceCStr, &shaderSourceSize);
			glCompileShader(shaderId);
			graphic_int compilationStatus;
			glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compilationStatus);
			if (compilationStatus != GL_TRUE)
			{
#ifndef NDEBUG
				graphic_int errorLogLength;
				glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &errorLogLength);
				std::vector<char> rawErrorLog;
				rawErrorLog.resize(errorLogLength);
				glGetShaderInfoLog(shaderId, errorLogLength, &errorLogLength, rawErrorLog.data());
				std::string_view errorLog{ rawErrorLog.data(), errorLogLength };
				std::cerr << errorLog << std::endl;
#endif
				glDeleteShader(shaderId);
				return std::nullopt;
			}

			return shaderId;
		}
	}

	constexpr char const* k_viewPosition = "u_viewPosition";
	constexpr char const* k_viewProjectionTransform = "u_viewProjectionTransform";
	constexpr char const* k_ambientColor = "u_ambientColor";
	constexpr char const* k_sunColor = "u_sunColor";
	constexpr char const* k_sunDirection = "u_sunDirection";
	constexpr char const* k_meshTransform = "u_meshTransform";
	constexpr char const* k_meshNormalTransform = "u_meshNormalTransform";
	constexpr char const* k_rigPose = "u_rigPose";

	void create_render_texture(
		glm::ivec2 const a_size
		, render_texture& a_renderTexture)
	{
		glGenTextures(1, &a_renderTexture.m_textureId);

		auto const width = a_size.x;
		auto const height = a_size.y;
		a_renderTexture.m_size = a_size;

		glBindTexture(GL_TEXTURE_2D, a_renderTexture.m_textureId);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		//glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_state.m_textureId);
		//glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height), GL_TRUE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


		glGenRenderbuffers(1, &a_renderTexture.m_renderbufferId);
		glBindRenderbuffer(GL_RENDERBUFFER, a_renderTexture.m_renderbufferId);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		//glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height));
		
		glGenFramebuffers(1, &a_renderTexture.m_framebufferId);
		glBindFramebuffer(GL_FRAMEBUFFER, a_renderTexture.m_framebufferId);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, a_renderTexture.m_textureId, 0);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_state.m_textureId, 0);
		glFramebufferRenderbuffer(
			GL_FRAMEBUFFER
			, GL_DEPTH_STENCIL_ATTACHMENT
			, GL_RENDERBUFFER
			, a_renderTexture.m_renderbufferId);

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}

	void destroy_render_texture(render_texture const& a_renderTexture)
	{
		glDeleteFramebuffers(1, &a_renderTexture.m_framebufferId);
		glDeleteRenderbuffers(1, &a_renderTexture.m_renderbufferId);
		glDeleteTextures(1, &a_renderTexture.m_textureId);
	}

	void create_program(
		std::string_view a_vertexShaderSource
		, std::string_view a_fragmentShaderSource
		, mesh_shader_program& a_program)
	{
		a_program.m_id = 0;

		auto const vertexShaderId = detail::create_shader(GL_VERTEX_SHADER, a_vertexShaderSource);
		if (!vertexShaderId)
		{
			return;
		}

		auto const fragmentShaderId = detail::create_shader(
			GL_FRAGMENT_SHADER, a_fragmentShaderSource);
		if (!fragmentShaderId)
		{
			glDeleteShader(*vertexShaderId);
			return;
		}

		a_program.m_id = glCreateProgram();
		glAttachShader(a_program.m_id, *vertexShaderId);
		glAttachShader(a_program.m_id, *fragmentShaderId);
		glLinkProgram(a_program.m_id);
		glDeleteShader(*vertexShaderId);
		glDeleteShader(*fragmentShaderId);
		graphic_int linkStatus;
		glGetProgramiv(a_program.m_id, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE)
		{
#ifndef NDEBUG
			graphic_int errorLogLength;
			glGetProgramiv(a_program.m_id, GL_INFO_LOG_LENGTH, &errorLogLength);
			std::vector<char> rawErrorLog;
			rawErrorLog.resize(errorLogLength);
			glGetProgramInfoLog(
				a_program.m_id, errorLogLength, &errorLogLength, rawErrorLog.data());
			std::string_view errorLog{ rawErrorLog.data(), errorLogLength };
			std::cerr << errorLog << std::endl;
#endif
			a_program.m_id = 0;
			return;
		}

		a_program.m_viewPositionLocation = glGetUniformLocation(a_program.m_id, k_viewPosition);
		a_program.m_viewProjectionTransformLocation = glGetUniformLocation(
			a_program.m_id, k_viewProjectionTransform);
		a_program.m_ambientColorLocation = glGetUniformLocation(a_program.m_id, k_ambientColor);
		a_program.m_sunColorLocation = glGetUniformLocation(a_program.m_id, k_sunColor);
		a_program.m_sunDirectionLocation = glGetUniformLocation(a_program.m_id, k_sunDirection);
		a_program.m_meshTransformLocation = glGetUniformLocation(a_program.m_id, k_meshTransform);
		a_program.m_meshNormalTransformLocation = glGetUniformLocation(
			a_program.m_id, k_meshNormalTransform);
		a_program.m_rigPoseLocation = glGetUniformLocation(a_program.m_id, k_rigPose);
	}

	void destroy_program(mesh_shader_program const& a_program)
	{
		glDeleteProgram(a_program.m_id);
	}

	template <typename TVertexData>
	void create_mesh(
		std::span<TVertexData const> const& a_vertices,
		std::span<triangle_data const> const& a_triangles,
		mesh& a_resource)
	{
		glGenVertexArrays(1, &a_resource.m_vao);
		glBindVertexArray(a_resource.m_vao);

		glGenBuffers(1, &a_resource.m_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, a_resource.m_vbo);
		glBufferData(
			GL_ARRAY_BUFFER
			, a_vertices.size() * sizeof(TVertexData)
			, a_vertices.data()
			, GL_STATIC_DRAW);

		glGenBuffers(1, &a_resource.m_ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, a_resource.m_ebo);
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER
			, a_triangles.size() * sizeof(triangle_data)
			, a_triangles.data()
			, GL_STATIC_DRAW);
		a_resource.m_triangleVertexCount = a_triangles.size();

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TVertexData)
			, reinterpret_cast<void*>(offsetof(TVertexData, m_position)));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TVertexData)
			, reinterpret_cast<void*>(offsetof(TVertexData, m_normal)));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TVertexData)
			, reinterpret_cast<void*>(offsetof(TVertexData, m_textureCoordinate)));

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(TVertexData)
			, reinterpret_cast<void*>(offsetof(TVertexData, m_tangent)));

		if constexpr (std::is_same_v<TVertexData, rigged_vertex>)
		{
			glEnableVertexAttribArray(4);
			glVertexAttribIPointer(4, 3, GL_UNSIGNED_BYTE, sizeof(TVertexData)
				, reinterpret_cast<void*>(offsetof(TVertexData, m_boneIndices)));

			glEnableVertexAttribArray(5);
			glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(TVertexData)
				, reinterpret_cast<void*>(offsetof(TVertexData, m_boneWeights)));
		}

		glBindVertexArray(0);
	}

	void destroy_mesh(mesh const& a_resource)
	{
		glDeleteBuffers(1, &a_resource.m_ebo);
		glDeleteBuffers(1, &a_resource.m_vbo);
		glDeleteVertexArrays(1, &a_resource.m_vao);
	}

	template <typename TChannel>
	void create_texture(image_view<TChannel> const& a_image, texture& a_texture)
	{

		glCreateTextures(GL_TEXTURE_2D, 1, &a_texture.m_id);
		glBindTexture(GL_TEXTURE_2D, a_texture.m_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		graphic_enum type;
		if constexpr (std::is_same_v<TChannel, std::uint8_t>)
		{
			type = GL_UNSIGNED_BYTE;
		}
		else
		{
			static_assert(false && "type unsupported");
		}

		glTexImage2D(
			GL_TEXTURE_2D
			, 0 // level
			, GL_RGBA // internal format
			, a_image.m_width
			, a_image.m_height
			, 0 // border
			, a_image.m_format
			, type
			, a_image.m_data);
	}

	void destroy_texture(texture const& a_texture)
	{
		glDeleteTextures(1, &a_texture.m_id);
	}
}
