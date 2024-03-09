#include <vob/aoe/rendering/systems/render_debug_mesh_system.h>

#include <vob/aoe/rendering/camera_util.h>
#include <vob/aoe/rendering/uniform_util.h>
#include <vob/aoe/rendering/resources/debug_mesh.h>

#include <vob/misc/std/message_macros.h>

#include <numbers>


namespace vob::aoegl
{
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
		const auto [viewPosition, viewProjectionTransform] = get_active_camera_settings(
			*m_directorWorldComponent, *m_cameraEntities, static_cast<float>(windowSize.x) / windowSize.y);
		uniform_util::set(program.m_viewPositionLocation, viewPosition);
		uniform_util::set(program.m_viewProjectionTransformLocation, viewProjectionTransform);

		// Update debug mesh
		{
			glBindVertexArray(debugRenderWorldComponent.m_vao);

			glBindBuffer(GL_ARRAY_BUFFER, debugRenderWorldComponent.m_vbo);
			glBufferData(
				GL_ARRAY_BUFFER,
				vertices.size() * sizeof(decltype(vertices.front())),
				vertices.data(),
				GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, debugRenderWorldComponent.m_ebo);
			glBufferData(
				GL_ELEMENT_ARRAY_BUFFER,
				lines.size() * sizeof(decltype(lines.front())),
				lines.data(),
				GL_STATIC_DRAW);
		}

		// Render debug mesh
		glDrawElements(GL_LINES, static_cast<int32_t>(lines.size() * 2), GL_UNSIGNED_INT, nullptr);

		m_debugMeshWorldComponent->clear_lines();
	}
}
