#pragma once

#include <vob/aoe/api.h>

#include "vob/aoe/rendering/CameraComponent.h"
#include "vob/aoe/rendering/CameraDirectorContext.h"
#include "vob/aoe/rendering/DebugCameraDirectorContext.h"

#include <vob/aoe/debug/DebugNameComponent.h>
#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include <vob/aoe/input/GameInputContext.h>


namespace vob::aoegl
{
	class VOB_AOE_API DebugCameraDirectorSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);
		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldGameControllerRef m_gameController;
		aoeng::EcsWorldContextRef<CameraDirectorContext> m_cameraDirectorCtx;
		aoeng::EcsWorldContextRef<aoein::GameInputContext const> m_gameInputCtx;
		aoeng::EcsWorldContextRef<DebugCameraDirectorContext const> m_debugCameraDirectorCtx;

		aoeng::EcsWorldViewRef<aoedb::DebugNameComponent const> m_debugNameEntities;
		aoeng::EcsWorldViewRef<CameraComponent const> m_cameraEntities;
	};
}
