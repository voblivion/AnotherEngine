#pragma once
#include <vob/aoe/ecs/world_data_provider.h>

#include <vob/aoe/common/_render/Cameracomponent.h>
#include <vob/aoe/common/_render/OpenGl.h>
#include <vob/aoe/common/_render/Utils.h>
#include <vob/aoe/common/_render/debugscene/DebugSceneRendercomponent.h>
#include <vob/aoe/common/window/WorldWindowcomponent.h>

namespace vob::aoe::common
{
	class DebugSceneRenderPass
	{
		using CameramanComponents = aoecs::ComponentTypeList<
			TransformComponent const
			, CameraComponent const
		>;
	public:
		// Constructor
		explicit DebugSceneRenderPass(aoecs::world_data_provider& a_wdp)
			: m_debugSceneRenderComponent{ a_wdp.get_world_component<DebugSceneRenderComponent>() }
			, m_worldWindowComponent{ a_wdp.get_world_component<WorldWindowComponent>() }
			, m_directorComponent{ a_wdp.get_world_component<DirectorComponent>() }
			, m_cameramanEntityList{ a_wdp.get_old_entity_view_list(*this, CameramanComponents{}) }
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
		_aoecs::entity_view_list<TransformComponent const, CameraComponent const> const& m_cameramanEntityList;
	};

}
