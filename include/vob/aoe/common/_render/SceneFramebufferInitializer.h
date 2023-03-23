#pragma once

#include <vob/aoe/ecs/world_data_provider.h>

#include <vob/aoe/common/_render/OpenGl.h>
#include <vob/aoe/common/_render/worldcomponents/SceneRendercomponent.h>

namespace vob::aoe::common
{
	class SceneFramebufferInitializer
	{
	public:
		// Constructor
		explicit SceneFramebufferInitializer(aoecs::world_data_provider& a_wdp)
            : m_sceneRenderComponent{ a_wdp.get_world_component<SceneRenderComponent>() }
		{}

		// Operators
		bool run() const
		{
			m_sceneRenderComponent.m_renderTextureManager.update();

			glBindFramebuffer(GL_FRAMEBUFFER, m_sceneRenderComponent.m_renderTexture->getFramebufferId());
			glClearColor(0.0f, 0.1f, 0.2f, 1.0f);
			glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			auto const windowSize = m_sceneRenderComponent.m_renderTexture->getSize();
            glViewport(0, 0, windowSize.x, windowSize.y);

			return true;
		}

	private:
		// Attributes
        SceneRenderComponent& m_sceneRenderComponent;
	};
}
