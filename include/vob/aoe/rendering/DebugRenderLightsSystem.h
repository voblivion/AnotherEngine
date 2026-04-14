#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/LightComponent.h>
#include <vob/aoe/rendering/DebugMeshContext.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include "vob/aoe/spacetime/PositionComponent.h"
#include "vob/aoe/spacetime/RotationComponent.h"


namespace vob::aoegl
{
	class VOB_AOE_API DebugRenderLightsSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoegl::DebugMeshContext> m_debugMeshContext;

		aoeng::EcsWorldViewRef<aoest::PositionComponent const, aoest::RotationComponent const, LightComponent const> m_lightEntities;
	};
}
