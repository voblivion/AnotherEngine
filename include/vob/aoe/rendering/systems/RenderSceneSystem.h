#pragma once

#include "vob/aoe/api.h"

#include "vob/aoe/rendering/components/CameraComponent.h"
#include "vob/aoe/rendering/components/InstancedModelsComponent.h"
#include "vob/aoe/rendering/components/LightComponent.h"
#include "vob/aoe/rendering/components/ModelComponent.h"
#include "vob/aoe/rendering/components/ModelTransformComponent.h"
#include "vob/aoe/rendering/contexts/CameraDirectorContext.h"
#include "vob/aoe/rendering/contexts/DebugMeshContext.h"
#include "vob/aoe/rendering/contexts/DebugProgramContext.h"
#include "vob/aoe/rendering/contexts/MaterialManagerContext.h"
#include "vob/aoe/rendering/contexts/RenderSceneContext.h"

#include "vob/aoe/engine/EcsWorldDataAccess.h"
#include "vob/aoe/spacetime/PositionComponent.h"
#include "vob/aoe/spacetime/RotationComponent.h"
#include "vob/aoe/spacetime/TimeContext.h"
#include "vob/aoe/window/WindowContext.h"


namespace vob::aoegl
{
	class VOB_AOE_API RenderSceneSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);
		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoest::TimeContext> m_timeContext;
		aoeng::EcsWorldContextRef<CameraDirectorContext> m_cameraDirectorContext;
		aoeng::EcsWorldContextRef<RenderSceneContext> m_renderSceneCtx;
		aoeng::EcsWorldContextRef<MaterialManagerContext> m_materialManagerContext;
		aoeng::EcsWorldContextRef<DebugProgramContext const> m_debugProgramContext;
		aoeng::EcsWorldContextRef<DebugMeshContext> m_debugMeshContext;
		aoeng::EcsWorldContextRef<aoewi::WindowContext> m_windowContext;
		aoeng::EcsWorldViewRef<aoest::PositionComponent const> m_focusEntities;
		aoeng::EcsWorldViewRef<aoest::PositionComponent const, aoest::RotationComponent const, CameraComponent const> m_cameraEntities;
		aoeng::EcsWorldViewRef<aoest::PositionComponent const, aoest::RotationComponent const, LightComponent const> m_lightEntities;
		aoeng::EcsWorldViewRef<aoest::PositionComponent const, aoest::RotationComponent const, StaticModelComponent const, ModelTransformComponent> m_staticModelEntities;
		aoeng::EcsWorldViewRef<aoest::PositionComponent const, aoest::RotationComponent const, RiggedModelComponent const, ModelTransformComponent> m_riggedModelEntities;
		aoeng::EcsWorldViewRef<aoest::PositionComponent const, aoest::RotationComponent const, InstancedModelsComponent const, ModelTransformComponent> m_instancedModelsEntities;

		// TODO: move out of system
		GraphicId m_debugProgram = k_invalidId;
		GraphicUniformLocation m_debugModeLoc = 0;
		GraphicId m_sceneVao = k_invalidId;
		GraphicId m_sceneVbo = k_invalidId;
	};
}
