#include <vob/aoe/rendering/systems/render_scene_system.h>

#include <vob/aoe/rendering/graphic_types.h>
#include <vob/aoe/rendering/resources/quad.h>
#include <vob/aoe/rendering/resources/vertex.h>
#include <vob/aoe/rendering/uniform_util.h>


namespace vob::aoegl
{
	render_scene_system::render_scene_system(aoeng::world_data_provider& a_wdp)
		: m_postProcessRenderWorldComponent{ a_wdp }
		, m_sceneTextureWorldComponent{ a_wdp }
		, m_windowWorldComponent{ a_wdp }
	{}

	void render_scene_system::update() const
	{
		auto const& program = m_postProcessRenderWorldComponent->m_postProcessProgram;

		// Use program
		glUseProgram(program.m_id);

		// Set post process uniforms
		uniform_util::set(program.m_windowSize, m_windowWorldComponent->m_window.get().get_size());

		// Allocate scene quad
		quad sceneQuad;
		glGenVertexArrays(1, &sceneQuad.m_vao);
		glBindVertexArray(sceneQuad.m_vao);

		glGenBuffers(1, &sceneQuad.m_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, sceneQuad.m_vbo);

		std::array<post_process_vertex, 6> vertices{
			post_process_vertex{ {-1.f, -1.f}, {.0f, 0.f} },
			post_process_vertex{ { 1.f, -1.f}, {1.f, 0.f} },
			post_process_vertex{ { 1.f,  1.f}, {1.f, 1.f} },
			post_process_vertex{ { 1.f,  1.f}, {1.f, 1.f} },
			post_process_vertex{ {-1.f,  1.f}, {0.f, 1.f} },
			post_process_vertex{ {-1.f, -1.f}, {0.f, 0.f} }};
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
			0
			, 2
			, GL_FLOAT
			, GL_FALSE
			, sizeof(post_process_vertex)
			, reinterpret_cast<void*>(offsetof(post_process_vertex, m_position))
		);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			1
			, 2
			, GL_FLOAT
			, GL_FALSE
			, sizeof(post_process_vertex)
			, reinterpret_cast<void*>(offsetof(post_process_vertex, m_textureCoord))
		);

		// Set scene texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_sceneTextureWorldComponent->m_sceneTexture.m_texture);

		// Render scene quad
		glBindVertexArray(sceneQuad.m_vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Deallocate scene quad
		glDeleteBuffers(1, &sceneQuad.m_vbo);
		glDeleteVertexArrays(1, &sceneQuad.m_vao);
	}
}
