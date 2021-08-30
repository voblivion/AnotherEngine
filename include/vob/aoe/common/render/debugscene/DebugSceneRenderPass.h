#pragma once
#include <vob/aoe/core/ecs/WorldDataProvider.h>

#include <vob/aoe/common/render/CameraComponent.h>
#include <vob/aoe/common/render/OpenGl.h>
#include <vob/aoe/common/render/Utils.h>
#include <vob/aoe/common/render/debugscene/DebugSceneRenderComponent.h>
#include <vob/aoe/common/window/WindowComponent.h>

namespace vob::aoe::common
{
	class DebugSceneRenderPass
	{
		using CameramanComponents = ecs::ComponentTypeList<
			TransformComponent const
			, CameraComponent const
		>;
	public:
		// Constructor
		explicit DebugSceneRenderPass(ecs::WorldDataProvider& a_wdp)
			: m_debugSceneRenderComponent{ a_wdp.getWorldComponentRef<DebugSceneRenderComponent>() }
			, m_windowComponent{ a_wdp.getWorldComponentRef<WindowComponent>() }
			, m_directorComponent{ a_wdp.getWorldComponentRef<DirectorComponent>() }
			, m_cameramanEntityList{ a_wdp.getEntityList(*this, CameramanComponents{}) }
		{}

		// Methods
		void run() const
		{
			if (m_debugSceneRenderComponent.m_shaderProgram == nullptr)
			{
				return;
			}
			auto const& shaderProgramHandle = *m_debugSceneRenderComponent.m_shaderProgram;

			m_debugSceneRenderComponent.m_debugSceneShaderProgramResourceManager.update();
			auto const& shaderProgram = *shaderProgramHandle;

			if (!initSceneShaderProgram(
				shaderProgram
				, m_windowComponent
				, m_directorComponent
				, m_cameramanEntityList
			))
			{
				return;
			}

			auto& mesh = m_debugSceneRenderComponent.m_debugMesh;
			mesh.create();
			mesh.update();
			mesh.render();
			mesh.reset();
			mesh.destroy();
		}

	private:
		// Attributes
		DebugSceneRenderComponent& m_debugSceneRenderComponent;
		WindowComponent& m_windowComponent;
		DirectorComponent& m_directorComponent;
		ecs::EntityList<TransformComponent const, CameraComponent const> const& m_cameramanEntityList;
	};

}
