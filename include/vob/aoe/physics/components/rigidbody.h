#pragma once

#include <vob/aoe/physics/material.h>
#include <vob/aoe/physics/math_util.h>

#include <vob/aoe/data/database.h>

#include <vob/misc/visitor/macros.h>

#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include <bullet/LinearMath/btDefaultMotionState.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>

#include <memory>


namespace vob::aoeph
{
	struct rigidbody
	{
		bool m_isStatic;
		float m_mass;
		glm::mat4 m_centerOfMassOffset;

		std::shared_ptr<btCollisionShape> m_collisionShape;
		std::shared_ptr<material const> m_material;

		std::unique_ptr<btRigidBody> m_instance = nullptr;
	};
}

namespace vob::misvi
{
	VOB_MISVI_ACCEPT(aoeph::rigidbody)
	{
		VOB_MISVI_NVP("Collision Shape", collisionShape);

		a_visitor.visit(nvp("Mass", a_value.m_mass));
		a_visitor.visit(nvp("Offset", a_value.m_centerOfMassOffset));
		glm::vec3 linearFactor;
		a_visitor.visit(nvp("Linear Factor", linearFactor));
		glm::vec3 angularFactor;
		a_visitor.visit(nvp("Angular Factor", angularFactor));

		aoeph::rigidbody& value = a_value;
		// TODO: remove? trying with unique_ptr<motion_state> for now
		/* value.m_motionState = std::make_shared<btDefaultMotionState>(
			btTransform::getIdentity(), aoeph::to_bt(glm::translate(glm::mat4{ 1.0f }, offset))); */

		// TODO: remove? done when adding component to world
		/*btVector3 inertia{0.0, 0.0, 0.0};
		if (mass != btScalar{ 0.0 } && value.m_collisionShape != nullptr)
		{
			value.m_collisionShape->calculateLocalInertia(mass, inertia);
		}

		value.m_instance = std::make_unique<btRigidBody>(
			mass, value.m_motionState.get(), value.m_collisionShape.get(), inertia);
		return true;*/
	}
}
