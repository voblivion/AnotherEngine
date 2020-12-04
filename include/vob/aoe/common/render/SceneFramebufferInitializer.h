#pragma once

#include <vob/aoe/core/ecs/WorldDataProvider.h>

#include <vob/aoe/common/render/OpenGl.h>
#include <vob/aoe/common/render/worldcomponents/SceneRenderComponent.h>

namespace vob::aoe::common
{
	class SceneFramebufferInitializer
	{
	public:
		// Constructor
		explicit SceneFramebufferInitializer(ecs::WorldDataProvider& a_wdp)
			: m_sceneRenderComponent{ a_wdp.getWorldComponentRef<SceneRenderComponent>() }
		{}

		// Operators
		bool run() const
		{
			m_sceneRenderComponent.m_renderTextureManager.update();

			glBindFramebuffer(GL_FRAMEBUFFER, m_sceneRenderComponent.m_renderTexture->getFramebufferId());
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glEnable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			return true;
		}

	private:
		// Attributes
		SceneRenderComponent& m_sceneRenderComponent;
	};
}
