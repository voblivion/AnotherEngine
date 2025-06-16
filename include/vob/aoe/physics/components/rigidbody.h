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
#include <numbers>


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

	struct aabb
	{
		glm::vec3 min;
		glm::vec3 max;
	};

	struct linear_velocity : public glm::vec3
	{
	};

	struct angular_velocity_local : public glm::vec3
	{

	};

	struct physx_material
	{
		float ellasticity = 100'000.0f;
		float restitution = 0.5f;
		float friction = 0.128f;
		float rolling_friction = 1.0f;

		float logRestitution = std::log(restitution);
		float zetaLow = std::sqrt(logRestitution * logRestitution / (logRestitution * logRestitution + std::numbers::pi_v<float> *std::numbers::pi_v<float>));
		float zetaHigh = zetaLow; // or 1.0f
	};

	struct dynamic_body
	{
		struct part
		{
			physx_material material;
			glm::vec3 position = glm::vec3{ 0.0f };
			glm::quat rotation = glm::quat();
			glm::vec3 radiuses = glm::vec3{ 1.0f };
		};

		std::vector<part> parts;

		glm::vec3 barycenter = glm::vec3{ 0.0f };

		float mass = 1.0f;
		glm::mat3 inertia = glm::mat3{ 1.0f };

		glm::vec3 force = glm::vec3{ 0.0f };
		glm::vec3 torque = glm::vec3{ 0.0f };

		aabb bounds;
	};

	struct triangle
	{
		glm::vec3 p0;
		glm::vec3 p1;
		glm::vec3 p2;
	};

	struct static_body
	{
		struct part
		{
			physx_material material;
			std::vector<triangle> triangles;
		};

		std::vector<part> parts;

		aabb bounds;
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
