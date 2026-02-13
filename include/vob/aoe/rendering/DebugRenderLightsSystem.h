#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/LightComponent.h>
#include <vob/aoe/rendering/DebugMeshContext.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include <vob/aoe/spacetime/Transform.h>


namespace vob::aoegl
{
	class VOB_AOE_API DebugRenderLightsSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoegl::DebugMeshContext> m_debugMeshContext;

		aoeng::EcsWorldViewRef<aoest::Position const, aoest::Rotation const, LightComponent const> m_lightEntities;
	};
}
