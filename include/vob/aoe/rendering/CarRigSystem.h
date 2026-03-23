#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/CarRigComponent.h>
#include <vob/aoe/rendering/ModelComponent.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include <vob/aoe/physics/CarControllerComponent.h>
#include <vob/aoe/physics/Collider.h>
#include <vob/aoe/spacetime/TimeContext.h>
#include <vob/aoe/spacetime/Transform.h>
#include <vob/aoe/spacetime/LinearVelocityComponent.h>


namespace vob::aoegl
{
	class VOB_AOE_API CarRigSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);
		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoest::TimeContext> m_timeContext;
		aoeng::EcsWorldViewRef<
			aoest::Rotation const,
			aoest::LinearVelocityComponent const,
			aoeph::CarCollider const,
			aoeph::CarControllerComponent const,
			RiggedModelComponent const,
			CarRigComponent> m_carEntities;
	};
}