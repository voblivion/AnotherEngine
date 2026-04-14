#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include <vob/aoe/rendering/DebugMeshContext.h>
#include <vob/aoe/spacetime/TimeContext.h>
#include "vob/aoe/spacetime/PositionComponent.h"
#include "vob/aoe/spacetime/RotationComponent.h"
#include <vob/aoe/spacetime/LinearVelocityComponent.h>
#include <vob/aoe/spacetime/SoftFollowComponent.h>


namespace vob::aoest
{
	class VOB_AOE_API SoftFollowSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoest::TimeContext> m_timeContext;
		aoeng::EcsWorldContextRef<aoegl::DebugMeshContext> m_debugMeshContext;

		aoeng::EcsWorldViewRef<aoest::PositionComponent const, aoest::RotationComponent const, aoest::LinearVelocityComponent const> m_softFollowableEntities;
		aoeng::EcsWorldViewRef<aoest::PositionComponent, aoest::RotationComponent, SoftFollowComponent> m_softFollowingEntities;
	};
}
