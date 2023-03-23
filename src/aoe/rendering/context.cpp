#include <vob/aoe/rendering/context.h>

#include <vob/aoe/rendering/color.h>
#include <vob/aoe/rendering/uniform_util.h>

#include <vob/aoe/spacetime/measures.h>

#include <glm/gtc/type_ptr.hpp>

#include <cassert>


namespace vob::aoegl
{
	void context::use_framebuffer(graphic_id a_framebuffer, rgba const& a_clearColor)
	{
		if (m_framebuffer != a_framebuffer)
		{
			m_framebuffer = a_framebuffer;
			glBindFramebuffer(GL_FRAMEBUFFER, a_framebuffer);
		}
		glClearColor(a_clearColor.r, a_clearColor.g, a_clearColor.b, a_clearColor.a);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void context::use_program(debug_program const& a_debugProgram)
	{
		glUseProgram(a_debugProgram.m_id);
		m_programContext = std::make_shared<debug_program_context>(a_debugProgram);
	}

	void context::use_program(mesh_program const& a_meshProgram)
	{
		glUseProgram(a_meshProgram.m_id);
		m_programContext = std::make_shared<mesh_program_context>(a_meshProgram);
	}

	void context::use_program(post_process_program const& a_postProcessProgram)
	{
		glUseProgram(a_postProcessProgram.m_id);
		m_programContext = std::make_shared<post_process_program_context>(a_postProcessProgram);
	}

	context::scene_program_context* context::get_scene_program_context()
	{
		return dynamic_cast<context::scene_program_context*>(m_programContext.get());
	}

	context::debug_program_context* context::get_debug_program_context()
	{
		return dynamic_cast<context::debug_program_context*>(m_programContext.get());
	}

	context::mesh_program_context* context::get_mesh_program_context()
	{
		return dynamic_cast<context::mesh_program_context*>(m_programContext.get());
	}

	context::post_process_program_context* context::get_post_process_program_context()
	{
		return dynamic_cast<context::post_process_program_context*>(m_programContext.get());
	}

	void context::scene_program_context::set_view_position(
		aoest::length_vector const& a_viewPosition)
	{
		assert(m_sceneProgram.m_id != 0);
		uniform_util::set(m_sceneProgram.m_viewPositionLocation, a_viewPosition);
	}

	void context::scene_program_context::set_view_projection_transform(
		glm::mat4 const& a_viewProjectionTransform)
	{
		assert(m_sceneProgram.m_id != 0);
		uniform_util::set(
			m_sceneProgram.m_viewProjectionTransformLocation, a_viewProjectionTransform);
	}

	void context::debug_program_context::render(debug_mesh const& a_debugMesh)
	{
		glBindVertexArray(a_debugMesh.m_vao);
		glDrawElements(
			GL_LINES
			, a_debugMesh.m_lineCount * 2
			, GL_UNSIGNED_INT
			, nullptr);
	}

	void context::mesh_program_context::set_material(material const& a_material)
	{
		assert(m_meshProgram.m_id != 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, a_material.m_albedo);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, a_material.m_normal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, a_material.m_metallicRoughness);
	}

	void context::mesh_program_context::set_sun_color(rgb const& a_ambientColor)
	{
		assert(m_meshProgram.m_id != 0);
		uniform_util::set(m_meshProgram.m_ambientColorLocation, a_ambientColor);
	}

	void context::mesh_program_context::set_sun_direction(
		aoest::length_vector const& a_sunDirection)
	{
		assert(m_meshProgram.m_id != 0);
		uniform_util::set(m_meshProgram.m_sunDirectionLocation, a_sunDirection);
	}

	void context::mesh_program_context::set_mesh_transform(glm::mat4 const& a_meshTransform)
	{
		assert(m_meshProgram.m_id != 0);
		uniform_util::set(m_meshProgram.m_meshTransformLocation, a_meshTransform);
	}

	void context::mesh_program_context::set_mesh_normal_transform(
		glm::mat4 const& a_meshNormalTransform)
	{
		assert(m_meshProgram.m_id != 0);
		uniform_util::set(m_meshProgram.m_meshNormalTransformLocation, a_meshNormalTransform);
	}

	void context::mesh_program_context::set_is_rigged(bool a_isRigged)
	{
		assert(m_meshProgram.m_id != 0);
		glUniform1i(m_meshProgram.m_isRiggedLocation, a_isRigged);
	}

	void context::mesh_program_context::set_rig_pose(std::span<glm::mat4 const> a_rigPose)
	{
		assert(m_meshProgram.m_id != 0);
		glUniformMatrix4fv(
			m_meshProgram.m_rigPoseLocation
			, static_cast<graphic_size>(std::min(a_rigPose.size(), m_meshProgram.k_maxRigSize))
			, GL_FALSE
			, a_rigPose.size() > 0 ? glm::value_ptr(a_rigPose[0]) : nullptr);
	}

	void context::post_process_program_context::set_window_size(glm::vec2 const& a_windowSize)
	{
		assert(m_postProcessProgram.m_id != 0);
		uniform_util::set(m_postProcessProgram.m_windowSize, a_windowSize);
	}

	void context::post_process_program_context::set_texture(graphic_id a_textureId)
	{
		assert(m_postProcessProgram.m_id != 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, a_textureId);
	}

	void context::post_process_program_context::render(quad const& a_quad)
	{
		assert(m_postProcessProgram.m_id != 0);
		glBindVertexArray(a_quad.m_vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}
