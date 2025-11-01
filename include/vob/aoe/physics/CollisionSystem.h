#pragma once

#include <vob/aoe/physics/Collider.h>
#include <vob/aoe/physics/CollisionContext.h>

#include <vob/aoe/spacetime/Transform.h>
#include <vob/aoe/spacetime/FixedRateTimeContext.h>
#include <vob/aoe/engine/EcsWorldDataAccess.h>

// TMP
#include <vob/aoe/rendering/DebugMeshContext.h>


namespace vob::aoeph
{
	class VOB_AOE_API CollisionSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoest::FixedRateTimeContext const> m_fixedRateTimeContext;
		aoeng::EcsWorldContextRef<CollisionContext> m_collisionContext;

		aoeng::EcsWorldViewRef<aoest::Position const, aoest::Rotation const, StaticCollider const> m_staticColliderEntities;
		aoeng::EcsWorldViewRef<aoest::Position, aoest::Rotation, CarCollider> m_carColliderEntities;

		aoeng::EcsWorldContextRef<aoegl::DebugMeshContext> m_debugMeshContext;
	};
}
