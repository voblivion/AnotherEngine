#pragma once

#include "resource.h"
#include "shader_program.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <span>


namespace vob::aoegl::render_util
{
	constexpr std::size_t k_maxRigSize = 64u;

	void init_render_texture(render_texture const& a_renderTexture)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, a_renderTexture.m_framebufferId);
		glClearColor(0.0f, 0.1f, 0.2f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, a_renderTexture.m_size.x, a_renderTexture.m_size.y);
	}

#pragma region set_uniform
	void set_uniform(
		shader_program const& a_program
		, graphic_uniform_location a_uniformLocation
		, std::int32_t const a_value)
	{
		glUniform1i(a_uniformLocation, a_value);
	}

	void set_uniform(
		shader_program const& a_program
		, graphic_uniform_location a_uniformLocation
		, std::uint32_t const a_value)
	{
		glUniform1ui(a_uniformLocation, a_value);
	}

	void set_uniform(
		shader_program const& a_program
		, graphic_uniform_location a_uniformLocation
		, glm::vec1 const& a_vector)
	{
		glUniform1f(a_uniformLocation, a_vector.x);
	}

	void set_uniform(
		shader_program const& a_program
		, graphic_uniform_location a_uniformLocation
		, glm::vec2 const& a_vector)
	{
		glUniform2f(a_uniformLocation, a_vector.x, a_vector.y);
	}

	void set_uniform(
		shader_program const& a_program
		, graphic_uniform_location a_uniformLocation
		, glm::vec3 const& a_vector)
	{
		glUniform3f(a_uniformLocation, a_vector.x, a_vector.y, a_vector.z);
	}

	void set_uniform(
		shader_program const& a_program
		, graphic_uniform_location a_uniformLocation
		, glm::vec4 const& a_vector)
	{
		glUniform4f(a_uniformLocation, a_vector.x, a_vector.y, a_vector.z, a_vector.w);
	}

	void set_uniform(
		shader_program const& a_program
		, graphic_uniform_location a_uniformLocation
		, glm::mat2 const& a_matrix)
	{
		glUniformMatrix2fv(a_uniformLocation, 1, false, glm::value_ptr(a_matrix));
	}

	void set_uniform(
		shader_program const& a_program
		, graphic_uniform_location a_uniformLocation
		, glm::mat3 const& a_matrix)
	{
		glUniformMatrix3fv(a_uniformLocation, 1, false, glm::value_ptr(a_matrix));
	}

	void set_uniform(
		shader_program const& a_program
		, graphic_uniform_location a_uniformLocation
		, glm::mat4 const& a_matrix)
	{
		glUniformMatrix4fv(a_uniformLocation, 1, false, glm::value_ptr(a_matrix));
	}
#pragma endregion

	void set_view_position(scene_shader_program const& a_program, glm::vec3 const& a_viewPosition)
	{
		set_uniform(a_program, a_program.m_viewPositionLocation, a_viewPosition);
	}

	void set_view_projection_transform(
		scene_shader_program const& a_program, glm::mat4 const& a_viewProjectionTransform)
	{
		set_uniform(
			a_program, a_program.m_viewProjectionTransformLocation, a_viewProjectionTransform);
	}

	void render(mesh_shader_program const& a_program, mesh const& a_mesh)
	{
		glBindVertexArray(a_mesh.m_vao);
		glDrawElements(GL_TRIANGLES, a_mesh.m_triangleVertexCount, GL_UNSIGNED_INT, nullptr);
	}

	void set_albedo(mesh_shader_program const& a_program, texture const& a_texture)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, a_texture.m_id);
	}

	void set_normal(mesh_shader_program const& a_program, texture const& a_texture)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, a_texture.m_id);
	}

	void set_metallic_roughness(mesh_shader_program const& a_program, texture const& a_texture)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, a_texture.m_id);
	}

	void set_ambient_color(mesh_shader_program const& a_program, glm::vec3 const& a_ambientColor)
	{
		set_uniform(a_program, a_program.m_ambientColorLocation, a_ambientColor);
	}

	void set_sun_color(mesh_shader_program const& a_program, glm::vec3 const& a_sunColor)
	{
		set_uniform(a_program, a_program.m_sunColorLocation, a_sunColor);
	}

	void set_sun_direction(mesh_shader_program const& a_program, glm::vec3 const& a_sunDirection)
	{
		set_uniform(a_program, a_program.m_sunDirectionLocation, a_sunDirection);
	}

	void set_mesh_transform(mesh_shader_program const& a_program, glm::mat4 const& a_meshTransform)
	{
		set_uniform(a_program, a_program.m_meshTransformLocation, a_meshTransform);
	}

	void set_mesh_normal_transform(
		mesh_shader_program const& a_program, glm::mat4 const& a_meshNormalTransform)
	{
		set_uniform(a_program, a_program.m_meshNormalTransformLocation, a_meshNormalTransform);
	}

	void set_rig_pose(mesh_shader_program const& a_program, std::span<glm::mat4> a_rigPose)
	{
		glUniformMatrix4fv(
			a_program.m_rigPoseLocation
			, std::min(a_rigPose.size(), k_maxRigSize)
			, GL_FALSE // transpose
			, a_rigPose.size() > 0 ? glm::value_ptr(a_rigPose[0]) : nullptr);
	}
}
