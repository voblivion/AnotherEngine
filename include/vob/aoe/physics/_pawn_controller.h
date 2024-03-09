#pragma once

#include <vob/misc/std/message_macros.h>

#include <bullet/BulletDynamics/Dynamics/btActionInterface.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>
#include <bullet/BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <bullet/LinearMath/btIDebugDraw.h>

#include <optional>
#include <numbers>
#include <vector>


namespace vob::aoeph
{
	struct cast_result_callback
		: public btCollisionWorld::ClosestConvexResultCallback
	{
		explicit cast_result_callback(btCollisionObject* a_self)
			: btCollisionWorld::ClosestConvexResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
			, m_self{ a_self }
		{}

		virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& a_convexResult, bool a_normalInWorldSpace) override
		{
			if (a_convexResult.m_hitCollisionObject == m_self)
			{
				return btScalar(1.0);
			}

			return ClosestConvexResultCallback::addSingleResult(a_convexResult, a_normalInWorldSpace);
		}

		btCollisionObject* m_self = nullptr;
		
	};

	class pawn_controller final : public btActionInterface
	{
	public:
		virtual void updateAction(btCollisionWorld* a_collisionWorld, btScalar a_dt) override;

		virtual void debugDraw(btIDebugDraw* debugDrawer) override
		{
			btCapsuleShape* capsuleShape = m_convexShape;

			btMatrix3x3 mat3 = btMatrix3x3::getIdentity();
			mat3.setEulerYPR(m_upRotation, btScalar(0.0), btScalar(0.0));
			btTransform transform(mat3, m_position);
			debugDrawer->drawCapsule(capsuleShape->getRadius(), capsuleShape->getHalfHeight(), 1, transform, btVector3(0, 0, 1));

			for (auto const& debugRay : m_debugRays)
			{
				debugDrawer->drawLine(debugRay.first, debugRay.second, btVector3(1, 1, 1));
			}
			for (auto const& debugHit : m_debugHits)
			{
				debugDrawer->drawSphere(debugHit, 0.1f, btVector3(1, 0, 0));
			}
		}

		// settings
		btVector3 m_up = { btScalar(0.0), btScalar(1.0), btScalar(0.0) };
		btScalar m_gravityStrength = btScalar(-9.81);
		btScalar m_maxWalkSpeed = btScalar(10.0);
		btScalar m_airFriction = btScalar(0.5);
		btScalar m_airControl = btScalar(0.1);
		btPairCachingGhostObject* m_ghostObject = new btPairCachingGhostObject();
		btCapsuleShape* m_convexShape = new btCapsuleShape(0.5f, 1.0f);
		btScalar m_maxStepUpLength = btScalar(0.1);
		btScalar m_maxStepDownLength = btScalar(0.1);
		btScalar m_maxSlopeCos = btCos(std::numbers::pi_v<btScalar> / 4);

		// inputs
		btVector3 m_jump = {};
		btVector3 m_horizontalAxis = {};
		
		// state
		bool m_isGrounded = false;
		btVector3 m_position = {};
		btScalar m_upRotation = {};
		btVector3 m_velocity = {};
		btVector3 m_groundNormal = { btScalar(0.0), btScalar(1.0), btScalar(0.0) };

		// debug
		std::vector<std::pair<btVector3, btVector3>> m_debugRays;
		std::vector<btVector3> m_debugHits;

		void updateGround(btCollisionWorld* a_collisionWorld, btScalar a_dt);
		void updateGroundJump(btCollisionWorld* a_collisionWorld, btScalar a_dt);
		void updateGroundWalk(btCollisionWorld* a_collisionWorld, btScalar a_dt);

		void updateAir(btCollisionWorld* a_collisionWorld, btScalar a_dt);
	};

	struct pawn_component
	{

	};
}
