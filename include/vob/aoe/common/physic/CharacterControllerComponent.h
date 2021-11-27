#pragma once
#include "vob/aoe/ecs/Component.h"
#include <optional>
#include <bullet/BulletDynamics/Character/btKinematicCharacterController.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>
#include <bullet/BulletCollision/CollisionShapes/btCapsuleShape.h>


namespace vob::aoe::common
{
	struct CharacterControllerComponent final
		: public aoecs::AComponent
	{
		std::optional<btKinematicCharacterController> m_kinematic;
		btPairCachingGhostObject m_ghost;
		// TODO should be convex shape
		btCapsuleShape m_capsule{ 0.25f, 1.0f };
	};
}

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ThisType>
	visitIfType<common::CharacterControllerComponent, ThisType> accept(VisitorType& a_visitor, ThisType& a_this)
	{
	}
}
