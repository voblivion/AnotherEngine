#pragma once

#include <vob/aoe/core/ecs/WorldDataProvider.h>

#include <vob/aoe/common/render/postprocess/PostProcessRenderComponent.h>
#include <vob/aoe/common/render/worldcomponents/SceneRenderComponent.h>


namespace vob::aoe::common
{
	class PostProcessRenderPass
	{
	public:
		// Constructor
		explicit PostProcessRenderPass(ecs::WorldDataProvider& a_wdp)
			: m_postProcessRenderComponent{ a_wdp.getWorldComponentRef<PostProcessRenderComponent>() }
			// , m_windowComponent{ a_wdp.getWorldComponentRef<WindowComponent>() }
			, m_directorComponent{ a_wdp.getWorldComponentRef<DirectorComponent>() }
			, m_sceneRenderComponent{ a_wdp.getWorldComponentRef<SceneRenderComponent>() }
		{
		
		}

		// Methods
		void run() const
		{
			if (!m_postProcessRenderComponent.m_shaderProgram.isValid())
			{
				return;
			}
			auto& shaderProgramHandle = *m_postProcessRenderComponent.m_shaderProgram;

			m_postProcessRenderComponent.m_shaderProgramResourceManager.update();
			auto& shaderProgram = *shaderProgramHandle;

			if (!shaderProgram.isReady())
			{
				return;
			}

			shaderProgram.use();

			auto& quad = m_postProcessRenderComponent.m_quad;
			quad.create();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_sceneRenderComponent.m_renderTexture->getTextureId());
			quad.render();
			quad.destroy();
		}

	private:
		PostProcessRenderComponent& m_postProcessRenderComponent;
		// WindowComponent& m_windowComponent;
		DirectorComponent& m_directorComponent;
		SceneRenderComponent& m_sceneRenderComponent;
	};
}
