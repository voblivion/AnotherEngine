#pragma once

#include "aoe/core/ecs/Component.h"
#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include "AMotionState.h"
#include <aoe/common/physic/ACollisionShape.h>
#include <aoe/core/standard/Cloneable.h>
#include "DefaultMotionState.h"
#include "PhysicMaterial.h"
#include <aoe/core/data/Handle.h>
#include <aoe/common/physic/BulletShapes.h>

namespace aoe::common
{
	struct RigidBodyComponent final
		: public vis::Aggregate<RigidBodyComponent, ecs::AComponent>
	{
		// Attributes
		std::optional<btRigidBody> m_rigidBody;
		btScalar m_mass{ 0.0 };
		type::Clone<btCollisionShape> m_collisionShape; // TODO should be handle?
		Vector3 m_linearFactor{ 1.0f };
		Vector3 m_angularFactor{ 1.0f };
		data::Handle<PhysicMaterial> m_physicMaterial;

		btDefaultMotionState m_motionState;
		Vector3 m_linearVelocity{ 0.0f };
		Quaternion m_angularVelocity{ Vector3{ 0.0f } };

		// Constructor
		explicit RigidBodyComponent(data::ADatabase& a_database
			, type::CloneCopier const& a_cloneCopier)
			: m_collisionShape{ a_cloneCopier }
			, m_physicMaterial{ a_database }
		{}

		// Methods
		friend class vis::Aggregate<RigidBodyComponent, ecs::AComponent>;
		template <typename VisitorType, typename ThisType>
		// ReSharper disable once CppMemberFunctionMayBeStatic
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
			a_visitor.visit(vis::nvp("Mass", a_this.m_mass));
			a_visitor.visit(vis::nvp("Shape", a_this.m_collisionShape));
			a_visitor.visit(vis::nvp("LinearFactor", a_this.m_linearFactor));
			a_visitor.visit(vis::nvp("AngularFactor", a_this.m_angularFactor));
			a_visitor.visit(vis::nvp("PhysicMaterial", a_this.m_physicMaterial));
		}
	};
}
