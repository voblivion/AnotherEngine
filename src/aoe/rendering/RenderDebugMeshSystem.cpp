#include <vob/aoe/rendering/RenderDebugMeshSystem.h>

#include <vob/aoe/rendering/CameraUtils.h>
#include <vob/aoe/rendering/UniformUtils.h>

#include <imgui.h>


namespace vob::aoegl
{
	void RenderDebugMeshSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_windowContext.init(a_wdar);
		m_cameraDirectorContext.init(a_wdar);
		m_debugRenderContext.init(a_wdar);
		m_debugMeshContext.init(a_wdar);
		m_cameraEntities.init(a_wdar);
	}

	void RenderDebugMeshSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		static bool kDisplay = false;
		if (ImGui::Begin("Debug Mesh"))
		{
			ImGui::Checkbox("Display", &kDisplay);
			ImGui::End();
		}

		auto& debugMeshContext = m_debugMeshContext.get(a_wdap);
		if (debugMeshContext.lines.empty())
		{
			return;
		}

		if (!kDisplay)
		{
			debugMeshContext.clear();
			return;
		}

		auto const& debugRenderContext = m_debugRenderContext.get(a_wdap);
		auto const& program = debugRenderContext.debugProgram;

		glClearDepth(1.0);
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glUseProgram(program.id);
		glLineWidth(2);

		// Set scene uniforms
		auto const windowSize = m_windowContext.get(a_wdap).window.get().getSize();
		auto const aspectRatio = static_cast<float>(windowSize.x) / windowSize.y;
		auto const [viewPosition, viewProjectionTransform] = getActiveCameraUniforms(
			m_cameraDirectorContext.get(a_wdap), m_cameraEntities.get(a_wdap), aspectRatio);
		setUniform(program.viewPositionLocation, viewPosition);
		setUniform(program.viewProjectionTransformLocation, viewProjectionTransform);

		// Update debug mesh
		{
			glBindVertexArray(debugRenderContext.vao);

			glBindBuffer(GL_ARRAY_BUFFER, debugRenderContext.vbo);
			glBufferData(
				GL_ARRAY_BUFFER,
				debugMeshContext.vertices.size() * sizeof(decltype(debugMeshContext.vertices.front())),
				debugMeshContext.vertices.data(),
				GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, debugRenderContext.ebo);
			glBufferData(
				GL_ELEMENT_ARRAY_BUFFER,
				debugMeshContext.lines.size() * sizeof(decltype(debugMeshContext.lines.front())),
				debugMeshContext.lines.data(),
				GL_STATIC_DRAW);
		}

		// Draw debug mesh
		glDrawElements(GL_LINES, static_cast<int32_t>(debugMeshContext.lines.size() * 2), GL_UNSIGNED_INT, nullptr);

		debugMeshContext.clear();
	}
}
