#pragma once

#include <vob/aoe/debug/GhostControllerComponent.h>
#include <vob/aoe/debug/IsControlledTag.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include <vob/aoe/input/InputBindings.h>
#include <vob/aoe/input/GameInputContext.h>
#include <vob/aoe/spacetime/Transform.h>
#include <vob/aoe/spacetime/TimeContext.h>


namespace vob::aoedb
{
	class VOB_AOE_API GhostControllerSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoein::GameInputContext const> m_gameInputCtx;
		aoeng::EcsWorldContextRef<aoest::TimeContext const> m_timeContext;
		aoeng::EcsWorldViewRef<aoest::Position, aoest::Rotation, GhostControllerComponent, IsControlledTag> m_ghostControllerEntities;
	};
}
