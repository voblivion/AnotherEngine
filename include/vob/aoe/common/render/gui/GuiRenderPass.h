#pragma once

#include <vob/aoe/ecs/WorldDataProvider.h>

#include <vob/aoe/common/render/gui/GuiRendercomponent.h>
#include <vob/aoe/common/render/gui/elements/EmptyElement.h>
#include <vob/aoe/common/render/gui/elements/TextElement.h>
#include <vob/aoe/common/render/gui/Canvascomponent.h>
#include <vob/aoe/common/time/WorldTimecomponent.h>
#include <vob/aoe/common/window/WorldWindowcomponent.h>

namespace vob::aoe::common
{
	class GuiRenderPass
	{
		using CanvasComponents = aoecs::ComponentTypeList<CanvasComponent>;
	public:
		// Constructor
		explicit GuiRenderPass(aoecs::WorldDataProvider& a_wdp)
			: m_guiRenderComponent{ a_wdp.getWorldComponentRef<GuiRenderComponent>() }
			, m_worldWindowComponent{ a_wdp.getWorldComponentRef<WorldWindowComponent const>() }
			, m_worldTimeComponent{ a_wdp.getWorldComponentRef<WorldTimeComponent const>() }
			, m_canvasEntityList{ a_wdp.getentity_view_list(*this, CanvasComponents{}) }
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
			auto const& window = m_worldWindowComponent.getWindow();
			auto const windowSize = glm::vec2{ window.getSize() };
			shaderProgram.setViewSize(windowSize);

			auto& quad = m_guiRenderComponent.m_guiRenderContext.m_quad;
			if (!quad->isReady())
			{
				quad->create();
			}
			m_guiRenderComponent.m_guiRenderContext.m_frameStartTime = m_worldTimeComponent.m_frameStartTime;

			for (auto const canvas : m_canvasEntityList)
			{
				auto& canvasComponent = canvas.get_component<CanvasComponent>();
				if (canvasComponent.m_rootElement != nullptr)
				{
					for (auto const& event : window.getPolledEvents())
					{
						canvasComponent.m_rootElement->onEvent(
							event,
							GuiTransform { glm::vec2{ 0.0f, 0.0f }, windowSize }
						);
					}

					canvasComponent.m_rootElement->render(
						shaderProgram
						, m_guiRenderComponent.m_guiRenderContext
						, GuiTransform{ glm::vec2{ 0.0f, 0.0f }, windowSize }
					);
				}
			}
		}

	private:
		mutable float t = 0.0f;
		GuiRenderComponent& m_guiRenderComponent;
		WorldWindowComponent const& m_worldWindowComponent;
		WorldTimeComponent const& m_worldTimeComponent;
		aoecs::entity_view_list<CanvasComponent> const& m_canvasEntityList;
	};
}
