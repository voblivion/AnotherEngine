#pragma once

#include "vob/aoe/api.h"

#include "vob/aoe/rendering/CameraComponent.h"
#include "vob/aoe/rendering/CameraDirectorContext.h"
#include "vob/aoe/rendering/DebugMeshContext.h"
#include "vob/aoe/rendering/DebugProgramContext.h"
#include "vob/aoe/rendering/LightComponent.h"
#include "vob/aoe/rendering/MaterialManagerContext.h"
#include "vob/aoe/rendering/ModelComponent.h"
#include "vob/aoe/rendering/PostProcessRenderContext.h"
#include "vob/aoe/rendering/RenderSceneContext.h"
#include "vob/aoe/rendering/SceneTextureContext.h"

#include "vob/aoe/engine/EcsWorldDataAccess.h"
#include "vob/aoe/spacetime/Transform.h"
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
		aoeng::EcsWorldContextRef<RenderSceneContext> m_renderSceneContext;
		aoeng::EcsWorldContextRef<MaterialManagerContext> m_materialManagerContext;
		aoeng::EcsWorldContextRef<DebugProgramContext const> m_debugProgramContext;
		aoeng::EcsWorldContextRef<DebugMeshContext> m_debugMeshContext;
		aoeng::EcsWorldContextRef<aoewi::WindowContext> m_windowContext;
		aoeng::EcsWorldViewRef<aoest::Position const, aoest::Rotation const, CameraComponent const> m_cameraEntities;
		aoeng::EcsWorldViewRef<aoest::Position const, aoest::Rotation const, LightComponent const> m_lightEntities;
		aoeng::EcsWorldViewRef<aoest::Position const, aoest::Rotation const, StaticModelComponent> m_staticModelEntities;
		aoeng::EcsWorldViewRef<aoest::Position const, aoest::Rotation const, RiggedModelComponent> m_riggedModelEntities;

		// TODO: move out of system
		GraphicId m_debugProgram = k_invalidId;
		GraphicUniformLocation m_debugModeLoc = 0;
		GraphicId m_sceneVao = k_invalidId;
		GraphicId m_sceneVbo = k_invalidId;
	};
}
