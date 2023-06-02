#include <vob/aoe/rendering/systems/render_debug_mesh_system.h>

#include <vob/aoe/rendering/uniform_util.h>

#include <vob/aoe/rendering/resources/debug_mesh.h>

#include <vob/misc/std/message_macros.h>

#include <numbers>


namespace vob::aoegl
{
	namespace
	{
#pragma message(VOB_MISTD_TODO "code duplicate")
		template <typename TDirectorWorldComponent, typename TCameraEntities>
		auto get_camera_settings(
			TDirectorWorldComponent const& a_directorWorldComponent,
			TCameraEntities const& a_cameraEntities)
		{
			auto const cameraEntityIt = a_cameraEntities.find(a_directorWorldComponent.m_activeCamera);
			if (cameraEntityIt == a_cameraEntities.end())
			{
				return std::make_tuple(glm::mat4{ 1.0f }, std::numbers::pi_v<float> / 2, 0.1f, 1000.0f);
			}

			auto [transformComponent, cameraComponent] = a_cameraEntities.get(*cameraEntityIt);
			return std::make_tuple(
				transformComponent.m_matrix
				, glm::radians(cameraComponent.m_fovDegree)
				, cameraComponent.m_nearClip
				, cameraComponent.m_farClip);
		}
	}

	render_debug_mesh_system::render_debug_mesh_system(aoeng::world_data_provider& a_wdp)
		: m_cameraEntities{ a_wdp }
		, m_windowWorldComponent{ a_wdp }
		, m_directorWorldComponent{ a_wdp }
		, m_debugRenderWorldComponent{ a_wdp }
		, m_debugSceneWorldComponent{ a_wdp }
	{}

	void render_debug_mesh_system::update() const
	{
		auto const& program = m_debugRenderWorldComponent->m_debugProgram;

		// Use program
		glUseProgram(program.m_id);

		// Set scene uniforms
		auto const windowSize = m_windowWorldComponent->m_window.get().get_size();
		const auto [transform, fov, nearClip, farClip] = get_camera_settings(
			*m_directorWorldComponent, *m_cameraEntities);
		uniform_util::set(program.m_viewPositionLocation, aoest::get_translation(transform));
		uniform_util::set(
			program.m_viewProjectionTransformLocation,
			glm::perspective(fov, static_cast<float>(windowSize.x) / windowSize.y, nearClip, farClip)
			* glm::inverse(transform));

		// Render debug mesh
		debug_mesh debugMesh;

		auto const& vertices = m_debugSceneWorldComponent->m_vertices;
		auto const& lines = m_debugSceneWorldComponent->m_lines;

		glCreateVertexArrays(1, &debugMesh.m_vao);
		glBindVertexArray(debugMesh.m_vao);

		glCreateBuffers(1, &debugMesh.m_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, debugMesh.m_vbo);
		glBufferData(
			GL_ARRAY_BUFFER
			, vertices.size() * sizeof(debug_vertex)
			, vertices.data()
			, GL_STATIC_DRAW);

		glCreateBuffers(1, &debugMesh.m_ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, debugMesh.m_ebo);
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER
			, lines.size() * sizeof(line)
			, lines.data()
			, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
			0
			, 3
			, GL_FLOAT
			, GL_FALSE
			, sizeof(debug_vertex)
			, reinterpret_cast<void*>(offsetof(debug_vertex, m_position)));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			1
			, 3
			, GL_FLOAT
			, GL_FALSE
			, sizeof(debug_vertex)
			, reinterpret_cast<void*>(offsetof(debug_vertex, m_color))
		);

		glBindVertexArray(debugMesh.m_vao);
		glDrawElements(
			GL_LINES
			, static_cast<std::uint32_t>(lines.size()) * 2
			, GL_UNSIGNED_INT
			, nullptr
		);

		glDeleteBuffers(1, &debugMesh.m_ebo);
		glDeleteBuffers(1, &debugMesh.m_vbo);
		glDeleteVertexArrays(1, &debugMesh.m_vao);
	}
}
