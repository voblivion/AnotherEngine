#pragma once
#include "ADynamicsWorldHolder.h"
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>


namespace vob::aoe::common
{
	class DefaultDynamicsWorldHolder final
		: public ADynamicsWorldHolder
	{
	public:
		btDynamicsWorld& getDynamicsWorld() override
		{
			return m_dynamicsWorld;
		}

		DefaultDynamicsWorldHolder()
		{
			auto& t = m_dynamicsWorld.getSolverInfo();
		}

		DefaultDynamicsWorldHolder(DefaultDynamicsWorldHolder const& a_other)
			: m_collisionConfiguration{ a_other.m_collisionConfiguration }
			, m_constraintSolver{ a_other.m_constraintSolver }
		{
			
		}

		btDefaultCollisionConfiguration m_collisionConfiguration;
		btCollisionDispatcher m_dispatcher{ &m_collisionConfiguration };
		btDbvtBroadphase m_pairCache{};
		btSequentialImpulseConstraintSolver m_constraintSolver{};
		btDiscreteDynamicsWorld m_dynamicsWorld{ &m_dispatcher, &m_pairCache
			, &m_constraintSolver, &m_collisionConfiguration };
	};
}
