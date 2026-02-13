#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/physics/Collider.h>
#include <vob/aoe/physics/CarControllerComponent.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include <vob/aoe/input/InputBindings.h>
#include <vob/aoe/input/GameInputContext.h>
#include <vob/aoe/spacetime/Transform.h>


namespace vob::aoeph
{
	class VOB_AOE_API CarControllerSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);
		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoein::GameInputContext const> m_gameInputCtx;
		aoeng::EcsWorldViewRef<aoest::Position const, aoest::Rotation const, CarCollider, CarControllerComponent> m_carEntities;
	};
}
