#pragma once

#include "vob/aoe/editor/GizmoContext.h"
#include "vob/aoe/editor/TranslateGizmoComponent.h"

#include "vob/aoe/engine/EcsWorldDataAccess.h"
#include "vob/aoe/rendering/contexts/DebugMeshContext.h"
#include "vob/aoe/spacetime/PositionComponent.h"
#include "vob/aoe/spacetime/RotationComponent.h"


namespace vob::aoedi
{
	class TranslateGizmoSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdp) const;

	private:
		aoeng::EcsWorldContextRef<aoegl::DebugMeshContext> m_debugMeshCtx;
		aoeng::EcsWorldContextRef<GizmoContext> m_gizmoCtx;
		aoeng::EcsWorldViewRef<aoest::PositionComponent, aoest::RotationComponent, TranslateGizmoComponent> m_gizmoEntities;
	};
}
