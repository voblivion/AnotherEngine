#pragma once
#include "aoe/core/ecs/Component.h"
#include "aoe/core/visitor/Aggregate.h"
#include <optional>
#include <bullet/BulletDynamics/Character/btKinematicCharacterController.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>
#include <bullet/BulletCollision/CollisionShapes/btCapsuleShape.h>

namespace aoe::common
{
	struct CharacterControllerComponent final
		: public vis::Aggregate<CharacterControllerComponent, ecs::AComponent>
	{
		std::optional<btKinematicCharacterController> m_kinematic;
		btPairCachingGhostObject m_ghost;
		// TODO should be convex shape
		btCapsuleShape m_capsule{ 0.25f, 1.0f };

		// Methods
		friend class vis::Aggregate<CharacterControllerComponent, ecs::AComponent>;
		template <typename VisitorType, typename ThisType>
		// ReSharper disable once CppMemberFunctionMayBeStatic
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{

		}
	};
}
