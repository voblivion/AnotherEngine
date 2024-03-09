#include <vob/aoe/physics/_pawn_controller.h>

namespace vob::aoeph
{
	void pawn_controller::updateAction(btCollisionWorld* a_collisionWorld, btScalar a_dt)
	{
		if (m_isGrounded)
		{
			updateGround(a_collisionWorld, a_dt);
		}
		else
		{
			updateAir(a_collisionWorld, a_dt);
		}
	}

	void pawn_controller::updateGround(btCollisionWorld* a_collisionWorld, btScalar a_dt)
	{
		if (!m_jump.fuzzyZero())
		{
			updateGroundJump(a_collisionWorld, a_dt);
		}
		else
		{
			updateGroundWalk(a_collisionWorld, a_dt);
		}
	}

	void pawn_controller::updateGroundJump(btCollisionWorld* a_collisionWorld, btScalar a_dt)
	{

	}

	void pawn_controller::updateGroundWalk(btCollisionWorld* a_collisionWorld, btScalar a_dt)
	{

	}

	void pawn_controller::updateAir(btCollisionWorld* a_collisionWorld, btScalar a_dt)
	{

	}
}
