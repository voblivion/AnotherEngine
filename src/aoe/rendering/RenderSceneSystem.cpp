#include <vob/aoe/rendering/RenderSceneSystem.h>

#include <vob/aoe/rendering/UniformUtils.h>


namespace vob::aoegl
{
	namespace
	{
		struct Quad
		{
			GraphicId vao = k_invalidId;
			GraphicId vbo = k_invalidId;
		};

		struct PostProcessVertex
		{
			glm::vec2 position;
			glm::vec2 textureCoord;
		};
	}

	void RenderSceneSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_postProcessRenderContext.init(a_wdar);
		m_sceneTextureContext.init(a_wdar);
		m_windowContext.init(a_wdar);
	}

	void RenderSceneSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto const& program = m_postProcessRenderContext.get(a_wdap).postProcessProgram;

		// Use program
		glUseProgram(program.id);

		// Set post process uniforms
		setUniform(program.windowSize, m_windowContext.get(a_wdap).window.get().getSize());

		// Allocate scene quad
		Quad sceneQuad;
		glGenVertexArrays(1, &sceneQuad.vao);
		glBindVertexArray(sceneQuad.vao);

		glGenBuffers(1, &sceneQuad.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, sceneQuad.vbo);

		std::array<PostProcessVertex, 6> vertices{
			PostProcessVertex{ {-1.f, -1.f}, {.0f, 0.f} },
			PostProcessVertex{ { 1.f, -1.f}, {1.f, 0.f} },
			PostProcessVertex{ { 1.f,  1.f}, {1.f, 1.f} },
			PostProcessVertex{ { 1.f,  1.f}, {1.f, 1.f} },
			PostProcessVertex{ {-1.f,  1.f}, {0.f, 1.f} },
			PostProcessVertex{ {-1.f, -1.f}, {0.f, 0.f} } };
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
			0
			, 2
			, GL_FLOAT
			, GL_FALSE
			, sizeof(PostProcessVertex)
			, reinterpret_cast<void*>(offsetof(PostProcessVertex, position)));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			1
			, 2
			, GL_FLOAT
			, GL_FALSE
			, sizeof(PostProcessVertex)
			, reinterpret_cast<void*>(offsetof(PostProcessVertex, textureCoord)));

		// Set scene texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_sceneTextureContext.get(a_wdap).texture.texture);

		// Render scene quad
		glBindVertexArray(sceneQuad.vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Deallocate scene quad
		glDeleteBuffers(1, &sceneQuad.vbo);
		glDeleteVertexArrays(1, &sceneQuad.vao);
	}
}
