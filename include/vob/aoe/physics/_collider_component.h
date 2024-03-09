#pragma once

// 1. same module
#include <vob/aoe/physics/material.h>

// 2. same project
#include <vob/aoe/data/database.h>

// 3. same owner
#include <vob/misc/std/message_macros.h>

// 4. other libraries
#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include <bullet/LinearMath/btDefaultMotionState.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/ext/quaternion_float.hpp>

// 5. standard library
#include <memory>

namespace vob::aoeph
{
	struct collider_component
	{
#pragma message(VOB_MISTD_TODO "split data from runtime")
		btScalar m_mass{ 0.0f };
		glm::vec3 m_offset{ 0.0f };
		glm::vec3 m_linearFactor{ 1.0f };
		glm::vec3 m_angularFactor{ 1.0f };
		std::shared_ptr<btCollisionShape const> m_shape;
		std::shared_ptr<material const> m_material;
	};
}

namespace vob::misvi
{
	VOB_MISVI_ACCEPT(aoeph::collider_component)
	{
		VOB_MISVI_NVP("Mass", mass);
		VOB_MISVI_NVP("Offset", offset);
		VOB_MISVI_NVP("LinearFactor", linearFactor);
		VOB_MISVI_NVP("AngularFactor", angularFactor);
		VOB_MISVI_NVP("Shape", shape);
		VOB_MISVI_NVP("Material", material);
		return true;
	}
}
