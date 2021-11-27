#pragma once
#include <vob/aoe/ecs/WorldDataProvider.h>

#include <vob/aoe/common/render/SceneFramebufferInitializer.h>
#include <vob/aoe/common/render/model/ModelRenderPass.h>
#include <vob/aoe/common/render/debugscene/DebugSceneRenderPass.h>

#include <vob/aoe/common/render/DefaultFramebufferInitializer.h>
#include <vob/aoe/common/render/postprocess/PostProcessRenderPass.h>
#include <vob/aoe/common/render/gui/GuiRenderPass.h>
#include <vob/aoe/common/render/BufferSwapper.h>

namespace vob::aoe::common
{
	/*template <typename TRenderPass>
	class RenderSystem
	{
	public:
		// Constructor
		explicit RenderSystem(aoecs::WorldDataProvider& a_wdp)
			: m_renderPass{ a_wdp }
		{}

		void update()
		{
			m_renderPass.run();
		}

	private:
		// Attributes
		TRenderPass m_renderPass;
	};*/

	class GameRenderSystem
	{
	public:
		// Constructor
		explicit GameRenderSystem(aoecs::WorldDataProvider& a_wdp)
			: m_sceneFramebufferInitializer{ a_wdp }
			, m_modelRenderPass{ a_wdp }
			, m_debugSceneRenderPass{ a_wdp }
			, m_defaultFramebufferInitializer{ a_wdp }
			, m_postProcessRenderPass{ a_wdp }
			, m_guiRenderPass{ a_wdp }
			, m_bufferSwapper{ a_wdp }
		{}

		void update() const
		{
			if (m_sceneFramebufferInitializer.run())
			{
				m_modelRenderPass.run();
				m_debugSceneRenderPass.run();
			}
			if (m_defaultFramebufferInitializer.run())
			{
				m_postProcessRenderPass.run();
				m_guiRenderPass.run();
			}

			m_bufferSwapper.run();
		}

	private:
		// Attributes
		SceneFramebufferInitializer m_sceneFramebufferInitializer;
		ModelRenderPass m_modelRenderPass;
		DebugSceneRenderPass m_debugSceneRenderPass;
		DefaultFramebufferInitializer<false, 255, 0, 127> m_defaultFramebufferInitializer;
		PostProcessRenderPass m_postProcessRenderPass;
		GuiRenderPass m_guiRenderPass;

		BufferSwapper m_bufferSwapper;
	};
}
