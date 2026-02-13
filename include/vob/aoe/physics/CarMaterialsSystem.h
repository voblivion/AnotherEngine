#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/physics/Collider.h>
#include <vob/aoe/physics/CarControllerComponent.h>
#include <vob/aoe/physics/CarMaterialsComponent.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include <vob/aoe/spacetime/TimeContext.h>
#include <vob/aoe/spacetime/Transform.h>


namespace vob::aoeph
{
	class VOB_AOE_API CarMaterialsSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);
		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoest::TimeContext> m_timeContext;
		aoeng::EcsWorldViewRef<aoest::Rotation, CarCollider, CarControllerComponent, CarMaterialsComponent> m_carEntities;
	};
}
