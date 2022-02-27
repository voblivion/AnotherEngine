#pragma once
#include <optional>
#include <bullet/BulletDynamics/Character/btKinematicCharacterController.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>
#include <bullet/BulletCollision/CollisionShapes/btCapsuleShape.h>


namespace vob::aoe::common
{
	struct CharacterControllerComponent final
	{
		std::optional<btKinematicCharacterController> m_kinematic;
		btPairCachingGhostObject m_ghost;
		// TODO should be convex shape
		btCapsuleShape m_capsule{ 0.25f, 1.0f };
	};
}

namespace vob::misvi
{
	template <typename VisitorType, typename ThisType>
	requires std::is_same_v<std::remove_cvref_t<ThisType>, aoe::common::CharacterControllerComponent>
	bool accept(VisitorType& a_visitor, ThisType& a_this)
	{
		return true;
	}
}
