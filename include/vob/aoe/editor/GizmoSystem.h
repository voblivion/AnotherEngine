#pragma once

#include "vob/aoe/editor/GizmoComponent.h"
#include "vob/aoe/editor/GizmoContext.h"
#include "vob/aoe/editor/GizmoSystem.h"

#include "vob/aoe/engine/EcsWorldDataAccess.h"
#include "vob/aoe/rendering/components/CameraComponent.h"
#include "vob/aoe/rendering/contexts/CameraDirectorContext.h"
#include "vob/aoe/spacetime/PositionComponent.h"
#include "vob/aoe/spacetime/RotationComponent.h"
#include "vob/aoe/window/WindowContext.h"


namespace vob::aoedi
{
	class  GizmoSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdp) const;

	private:
		aoeng::EcsWorldContextRef<aoewi::WindowContext> m_windowCtx;
		aoeng::EcsWorldContextRef<aoegl::CameraDirectorContext> m_cameraDirectorCtx;
		aoeng::EcsWorldContextRef<GizmoContext> m_gizmoCtx;

		aoeng::EcsWorldViewRef<aoest::PositionComponent const, aoest::RotationComponent const, aoegl::CameraComponent const> m_cameraEntities;
		aoeng::EcsWorldViewRef<aoedi::GizmoComponent> m_gizmoEntities;
	};
}
