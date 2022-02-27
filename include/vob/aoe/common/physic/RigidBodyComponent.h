#pragma once

#include "AMotionState.h"
#include "DefaultMotionState.h"
#include "PhysicMaterial.h"
#include "ACollisionShape.h"
#include "BulletShapes.h"


#include <vob/misc/visitor/name_value_pair.h>

#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>

#include <memory>


namespace vob::aoe::common
{
	struct RigidBodyComponent final
	{
		// Attributes
		btScalar m_mass{ 0.0 };
		type::dynamic_type_clone<ACollisionShape> m_collisionShape;
		glm::vec3 m_linearFactor{ 1.0f };
		glm::vec3 m_angularFactor{ 1.0f };
		std::shared_ptr<PhysicMaterial const> m_physicMaterial;

		glm::vec3 m_linearVelocity{ 0.0f };
		glm::quat m_angularVelocity{ glm::vec3{ 0.0f } };

		glm::vec3 m_offset{ 0.0f };

		// shared but different from one entity in the world to another
		std::shared_ptr<btRigidBody> m_rigidBody;
		std::shared_ptr<btDefaultMotionState> m_motionState;

		// Constructor
		explicit RigidBodyComponent(type::dynamic_type_clone_copier const& a_cloner)
			: m_collisionShape{ a_cloner }
		{}
	};
}

namespace vob::misvi
{
	template <typename VisitorType, typename ThisType>
	requires std::is_same_v<std::remove_cvref_t<ThisType>, aoe::common::RigidBodyComponent>
	bool accept(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(misvi::nvp("Mass", a_this.m_mass));
		a_visitor.visit(misvi::nvp("Shape", a_this.m_collisionShape));
		a_visitor.visit(misvi::nvp("LinearFactor", a_this.m_linearFactor));
		a_visitor.visit(misvi::nvp("AngularFactor", a_this.m_angularFactor));
		a_visitor.visit(misvi::nvp("PhysicMaterial", a_this.m_physicMaterial));
		a_visitor.visit(misvi::nvp("Offset", a_this.m_offset));
		return true;
	}
}
