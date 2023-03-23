#pragma once

#include <vob/aoe/ecs/world_data_provider.h>

#include <vob/aoe/common/_render/postprocess/PostProcessRendercomponent.h>
#include <vob/aoe/common/_render/worldcomponents/SceneRendercomponent.h>


namespace vob::aoe::common
{
	class PostProcessRenderPass
	{
	public:
		// Constructor
		explicit PostProcessRenderPass(aoecs::world_data_provider& a_wdp)
			: m_postProcessRenderComponent{ a_wdp.get_world_component<PostProcessRenderComponent>() }
			, m_worldWindowComponent{ a_wdp.get_world_component<WorldWindowComponent>() }
			, m_directorComponent{ a_wdp.get_world_component<DirectorComponent>() }
			, m_sceneRenderComponent{ a_wdp.get_world_component<SceneRenderComponent>() }
		{
		
		}

		// Methods
		void run() const
		{
			if (m_postProcessRenderComponent.m_shaderProgram == nullptr)
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
            const auto& window = m_worldWindowComponent.getWindow();
            auto const windowSize = window.getSize();
			shaderProgram.setUniform(
				shaderProgram.getWindowSizeLocation()
				, glm::vec2{ windowSize }
			);

			auto& quad = m_postProcessRenderComponent.m_quad;
			quad.create();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_sceneRenderComponent.m_renderTexture->getTextureId());
			quad.render();
			quad.destroy();
		}

	private:
		mutable unsigned m_currentPostProcess = 0;
		mutable float m_brightness = 0.0f;
		mutable float m_saturation = 0.0f;
		PostProcessRenderComponent& m_postProcessRenderComponent;
		WorldWindowComponent& m_worldWindowComponent;
		DirectorComponent& m_directorComponent;
		SceneRenderComponent& m_sceneRenderComponent;
	};
}
