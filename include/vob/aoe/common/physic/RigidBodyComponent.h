#pragma once

#include <memory>

#include "vob/aoe/core/ecs/Component.h"
#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include "AMotionState.h"
#include <vob/aoe/common/physic/ACollisionShape.h>
#include "DefaultMotionState.h"
#include "PhysicMaterial.h"
#include <vob/aoe/common/physic/BulletShapes.h>

namespace vob::aoe::common
{
	struct RigidBodyComponent final
		: public ecs::AComponent
	{
		// Attributes
		btScalar m_mass{ 0.0 };
		type::Cloneable<ACollisionShape> m_collisionShape;
		vec3 m_linearFactor{ 1.0f };
		vec3 m_angularFactor{ 1.0f };
		std::shared_ptr<PhysicMaterial const> m_physicMaterial;

		vec3 m_linearVelocity{ 0.0f };
		quat m_angularVelocity{ vec3{ 0.0f } };

		vec3 m_offset{ 0.0f };

		// shared but different from one entity in the world to another
		std::shared_ptr<btRigidBody> m_rigidBody;
		std::shared_ptr<btDefaultMotionState> m_motionState;

		// Constructor
		explicit RigidBodyComponent(type::Cloner<> const& a_cloner)
			: m_collisionShape{ a_cloner }
		{}
	};
}

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ThisType>
	visitIfType<common::RigidBodyComponent, ThisType> accept(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(vis::nvp("Mass", a_this.m_mass));
		a_visitor.visit(vis::nvp("Shape", a_this.m_collisionShape));
		a_visitor.visit(vis::nvp("LinearFactor", a_this.m_linearFactor));
		a_visitor.visit(vis::nvp("AngularFactor", a_this.m_angularFactor));
		a_visitor.visit(vis::nvp("PhysicMaterial", a_this.m_physicMaterial));
		a_visitor.visit(vis::nvp("Offset", a_this.m_offset));
	}
}
