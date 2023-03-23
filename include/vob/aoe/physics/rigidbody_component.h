#pragma once

#include <vob/aoe/physics/scalar.h>
#include <vob/aoe/physics/material.h>

#include <vob/aoe/data/database.h>

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include <bullet/LinearMath/btDefaultMotionState.h>

#include <memory>

namespace vob::aoeph
{
	struct rigidbody_component
	{
		scalar m_mass{ 0.0f };
		glm::vec3 m_linearFactor{ 1.0f };
		glm::vec3 m_angularFactor{ 1.0f };
		std::shared_ptr<material const> m_physicMaterial;
		// collisionShape
		
		glm::vec3 m_linearVelocity{ 0.0f };
		glm::quat m_angularVelocity{ glm::vec3{ 0.0f } };

		glm::vec3 m_offset{ 0.0f };

		std::shared_ptr<btRigidBody> m_rigidBody;
		std::shared_ptr<btDefaultMotionState> m_motionState;
	};
}

namespace vob::misvi
{
	VOB_MISVI_ACCEPT(aoeph::rigidbody_component)
	{
		VOB_MISVI_NVP("Mass", mass);
		VOB_MISVI_NVP("LinearFactor", linearFactor);
		VOB_MISVI_NVP("AngularFactor", angularFactor);
		VOB_MISVI_NVP("PhysicMaterial", physicMaterial);
		// VOB_MISVI_NVP("Shape", collisionShape);
		VOB_MISVI_NVP("Offset", offset);
		return true;
	}
}
