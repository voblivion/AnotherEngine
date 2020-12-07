#pragma once

#include <vob/aoe/core/ecs/WorldDataProvider.h>

#include <vob/aoe/common/render/gui/GuiRenderComponent.h>
#include <vob/aoe/common/render/gui/elements/EmptyElement.h>
#include <vob/aoe/common/render/gui/elements/TextElement.h>
#include <vob/aoe/common/render/gui/CanvasComponent.h>

namespace vob::aoe::common
{
	class GuiRenderPass
	{
		using CanvasComponents = ecs::ComponentTypeList<CanvasComponent const>;
	public:
		// Constructor
		explicit GuiRenderPass(ecs::WorldDataProvider& a_wdp)
			: m_guiRenderComponent{ a_wdp.getWorldComponentRef<GuiRenderComponent>() }
			, m_canvasEntityList{ a_wdp.getEntityList(*this, CanvasComponents{}) }
		{}

		// Methods
		void run() const
		{
			if (!m_guiRenderComponent.m_shaderProgram.isValid())
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
			shaderProgram.setViewSize(vec2{ 2048.f, 1024.f });

			auto& quad = m_guiRenderComponent.m_guiRenderContext.m_quad;
			if (!quad->isReady())
			{
				quad->create();
			}

			for (auto const canvas : m_canvasEntityList)
			{
				auto const& canvasComponent = canvas.getComponent<CanvasComponent>();
				if (canvasComponent.m_rootElement != nullptr)
				{
					canvasComponent.m_rootElement->render(
						shaderProgram
						, m_guiRenderComponent.m_guiRenderContext
						, GuiTransform{ vec2{ 0.0f, 0.0f }, canvasComponent.m_size }
					);
				}
			}

			//EmptyElement element{};
			//element.m_margin = vec4{ 15.0f, 15.0f, 15.0f, 15.0f };
			//element.m_borderWidth = vec4{ 5.0f, 5.0f, 5.0f, 5.0f };
			//element.m_innerCornerRadius = vec4{ 20.0f, 0.0f, 0.0f, 0.0f };
			//element.render(shaderProgram, m_guiRenderComponent.m_guiRenderContext, GuiTransform{ vec2{0.0f, 0.0f}, vec2{200.0f, 500.0f} });

			//auto& textElement = m_guiRenderComponent.m_textElementTest;
			//textElement.setSize(5 + 10 * (1 + std::sin(t)));
			//t += 0.0005f;
			//if (!textElement.getFont().isValid())
			//{
			//	textElement.setFont(data::Handle<Font>{ textElement.getFont().getDatabase(), 9 });
			//}
			//textElement.setText(
			//	"Bonjour je m'appelle Victor et je suis en train de creer "
			//	"un super text-renderer :)"
			//);
			//textElement.render(shaderProgram, m_guiRenderComponent.m_guiRenderContext, GuiTransform{ vec2{20.0f, 20.0f}, vec2{160.0f, 460.0f} });
		}

	private:
		mutable float t = 0.0f;
		GuiRenderComponent& m_guiRenderComponent;
		ecs::EntityList<CanvasComponent const> const& m_canvasEntityList;

	};
}
