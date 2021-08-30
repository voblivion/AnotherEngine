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
			, m_windowComponent{ a_wdp.getWorldComponentRef<WindowComponent>() }
			, m_directorComponent{ a_wdp.getWorldComponentRef<DirectorComponent>() }
			, m_sceneRenderComponent{ a_wdp.getWorldComponentRef<SceneRenderComponent>() }
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
            const auto& window = m_windowComponent.getWindow();
            auto const windowSize = window.getSize();
			shaderProgram.setUniform(
				shaderProgram.getWindowSizeLocation()
				, vec2{ windowSize }
			);

			for (auto const& event : window.getPolledEvents())
			{
				std::visit([this, &shaderProgram](auto const& a_event)
				{
					using EventType = std::decay_t<decltype(a_event)>;
					if constexpr (std::is_same_v<EventType, KeyEvent>)
					{
						auto const& keyEvent = static_cast<KeyEvent const&>(a_event);

						if (keyEvent.m_action == KeyEvent::Action::Release
							|| !keyEvent.m_modifierMask.hasModifier(Modifier::Control))
						{
							return;
						}

						// PS1/CRT post-process <3
						switch (keyEvent.m_keyCode)
						{
                        case GLFW_KEY_P:
                            m_currentPostProcess = (m_currentPostProcess + 1) % 2;
                            shaderProgram.setUniform(
                                shaderProgram.getPostProcessTypeLocation()
                                , m_currentPostProcess
                            );
							break;
                        case GLFW_KEY_UP:
                            m_brightness += 0.1f;
							shaderProgram.setUniform(
                                shaderProgram.getBrightnessLocation()
                                , m_brightness
                            );
							break;
                        case GLFW_KEY_DOWN:
							m_brightness -= 0.1f;
                            shaderProgram.setUniform(
                                shaderProgram.getBrightnessLocation()
                                , m_brightness
                            );
							break;
						case GLFW_KEY_LEFT:
							m_saturation -= 0.1f;
							shaderProgram.setUniform(
								shaderProgram.getSaturationLocation()
								, m_saturation
							);
                            break;
                        case GLFW_KEY_RIGHT:
                            m_saturation += 0.1f;
                            shaderProgram.setUniform(
                                shaderProgram.getSaturationLocation()
                                , m_saturation
                            );
                            break;
						}

					}
				}, event);
			}

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
		WindowComponent& m_windowComponent;
		DirectorComponent& m_directorComponent;
		SceneRenderComponent& m_sceneRenderComponent;
	};
}
