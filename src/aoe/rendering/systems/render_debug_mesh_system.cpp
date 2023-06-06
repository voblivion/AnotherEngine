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
		, m_debugMeshWorldComponent{ a_wdp }
	{
		// vao
		{
			glCreateVertexArrays(1, &(m_debugRenderWorldComponent->m_vao));
			glBindVertexArray(m_debugRenderWorldComponent->m_vao);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);

			// position
			glEnableVertexAttribArray(0);
			glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
			glVertexAttribBinding(0, 0);

			// color
			glEnableVertexAttribArray(1);
			glVertexAttribFormat(1, 4, GL_FLOAT, GL_FALSE, 0);
			glVertexAttribBinding(1, 1);
		}
		// vbo
		{
			graphic_id vbo;
			glCreateBuffers(1, &(m_debugRenderWorldComponent->m_vbo));
			glBindVertexBuffer(
				0,
				m_debugRenderWorldComponent->m_vbo,
				offsetof(debug_vertex, m_position),
				sizeof(debug_vertex));
			glBindVertexBuffer(
				1,
				m_debugRenderWorldComponent->m_vbo,
				offsetof(debug_vertex, m_color),
				sizeof(debug_vertex));
		}
		// ebo
		{
			glCreateBuffers(1, &(m_debugRenderWorldComponent->m_ebo));
		}
	}

	void render_debug_mesh_system::update() const
	{
		auto& vertices = m_debugMeshWorldComponent->m_vertices;
		auto& lines = m_debugMeshWorldComponent->m_lines;
		if (vertices.empty() || lines.empty())
		{
			return;
		}

		auto& debugRenderWorldComponent = *m_debugRenderWorldComponent;
		auto const& program = debugRenderWorldComponent.m_debugProgram;

		// Use program
		glUseProgram(program.m_id);

		// Set scene uniforms
		auto const windowSize = m_windowWorldComponent->m_window.get().get_size();
		const auto [transform, fov, nearClip, farClip] = get_camera_settings(
			*m_directorWorldComponent, *m_cameraEntities);
		uniform_util::set(program.m_viewPositionLocation, aoest::get_translation(transform));
		uniform_util::set(
			program.m_viewProjectionTransformLocation,
			glm::perspective(
				fov,
				static_cast<float>(windowSize.x) / windowSize.y,
				nearClip,
				farClip) * glm::inverse(transform));

		// Render debug mesh
		{
			glBindVertexArray(debugRenderWorldComponent.m_vao);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);

			// vbo
			glBindBuffer(GL_ARRAY_BUFFER, debugRenderWorldComponent.m_vbo);
			glBufferData(
				GL_ARRAY_BUFFER,
				vertices.size() * sizeof(decltype(vertices.front())),
				vertices.data(),
				GL_STATIC_DRAW);

			// ebo
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, debugRenderWorldComponent.m_ebo);
			glBufferData(
				GL_ELEMENT_ARRAY_BUFFER,
				lines.size() * sizeof(decltype(lines.front())),
				lines.data(),
				GL_STATIC_DRAW);

			// draw
			glDrawElements(GL_LINES, static_cast<int32_t>(lines.size() * 2), GL_UNSIGNED_INT, nullptr);
		}

		m_debugMeshWorldComponent->clear_lines();
	}
}
