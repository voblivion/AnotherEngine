#pragma once
#include <vob/aoe/core/ecs/WorldDataProvider.h>

#include <vob/aoe/common/render/CameraComponent.h>
#include <vob/aoe/common/render/OpenGl.h>
#include <vob/aoe/common/render/Utils.h>
#include <vob/aoe/common/render/debugscene/DebugSceneRenderComponent.h>
#include <vob/aoe/common/window/WorldWindowComponent.h>

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
			, m_worldWindowComponent{ a_wdp.getWorldComponentRef<WorldWindowComponent>() }
			, m_directorComponent{ a_wdp.getWorldComponentRef<DirectorComponent>() }
			, m_cameramanEntityList{ a_wdp.getEntityViewList(*this, CameramanComponents{}) }
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
				, m_worldWindowComponent
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
		WorldWindowComponent& m_worldWindowComponent;
		DirectorComponent& m_directorComponent;
		ecs::EntityViewList<TransformComponent const, CameraComponent const> const& m_cameramanEntityList;
	};

}
