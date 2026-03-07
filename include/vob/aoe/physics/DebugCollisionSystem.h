#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/physics/Collider.h>

#include <vob/aoe/debug/DebugNameComponent.h>
#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include <vob/aoe/rendering/DebugMeshContext.h>
#include <vob/aoe/spacetime/Transform.h>


namespace vob::aoeph
{
	class VOB_AOE_API DebugCollisionSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);

		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoegl::DebugMeshContext> m_debugMeshContext;

		aoeng::EcsWorldViewRef<aoedb::DebugNameComponent const> m_debugNameEntities;
		aoeng::EcsWorldViewRef<aoest::Position const, aoest::Rotation const, StaticCollider const> m_staticColliderEntities;
		aoeng::EcsWorldViewRef<aoest::Position const, aoest::Rotation const, CarCollider const> m_carColliderEntities;
	};
}
