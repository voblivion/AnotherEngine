#pragma once

#include <vob/aoe/core/ecs/WorldDataProvider.h>

#include <vob/aoe/common/render/gui/GuiRenderComponent.h>
#include <vob/aoe/common/render/gui/elements/EmptyElement.h>
#include <vob/aoe/common/render/gui/elements/TextElement.h>
#include <vob/aoe/common/render/gui/CanvasComponent.h>
#include <vob/aoe/common/time/TimeComponent.h>
#include <vob/aoe/common/window/WindowComponent.h>

namespace vob::aoe::common
{
	class GuiRenderPass
	{
		using CanvasComponents = ecs::ComponentTypeList<CanvasComponent const>;
	public:
		// Constructor
		explicit GuiRenderPass(ecs::WorldDataProvider& a_wdp)
			: m_guiRenderComponent{ a_wdp.getWorldComponentRef<GuiRenderComponent>() }
			, m_windowComponent{ a_wdp.getWorldComponentRef<WindowComponent const>() }
			, m_worldTimeComponent{ a_wdp.getWorldComponentRef<TimeComponent const>() }
			, m_canvasEntityList{ a_wdp.getEntityViewList(*this, CanvasComponents{}) }
		{}

		// Methods
		void run() const
		{
			if (m_guiRenderComponent.m_shaderProgram == nullptr)
			{
				return;
			}

			auto& shaderProgramHandle = *m_guiRenderComponent.m_shaderProgram;

			m_guiRenderComponent.m_guiMeshResourceManager.update();
			m_guiRenderComponent.m_shaderProgramResourceManager.update();
			m_guiRenderComponent.m_textureResourceManager.update();
			auto& shaderProgram = *shaderProgramHandle;

			if (!shaderProgram.isReady())
			{
				return;
			}

			shaderProgram.use();
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // TODO
			auto const& window = m_windowComponent.getWindow();
			auto const windowSize = vec2{ window.getSize() };
			shaderProgram.setViewSize(windowSize);

			auto& quad = m_guiRenderComponent.m_guiRenderContext.m_quad;
			if (!quad->isReady())
			{
				quad->create();
			}
			m_guiRenderComponent.m_guiRenderContext.m_frameStartTime = m_worldTimeComponent.m_frameStartTime;

			for (auto const canvas : m_canvasEntityList)
			{
				auto const& canvasComponent = canvas.getComponent<CanvasComponent>();
				if (canvasComponent.m_rootElement != nullptr)
				{
					for (auto const& event : window.getPolledEvents())
					{
						canvasComponent.m_rootElement->onEvent(
							event,
							GuiTransform { vec2{ 0.0f, 0.0f }, windowSize }
						);
					}

					canvasComponent.m_rootElement->render(
						shaderProgram
						, m_guiRenderComponent.m_guiRenderContext
						, GuiTransform{ vec2{ 0.0f, 0.0f }, windowSize }
					);
				}
			}
		}

	private:
		mutable float t = 0.0f;
		GuiRenderComponent& m_guiRenderComponent;
		WindowComponent const& m_windowComponent;
		TimeComponent const& m_worldTimeComponent;
		ecs::EntityViewList<CanvasComponent const> const& m_canvasEntityList;
	};
}
