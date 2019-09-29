#pragma once

#include "aoe/core/visitor/Aggregate.h"
#include <LinearMath/btScalar.h>
#include <aoe/core/visitor/Utils.h>


namespace aoe
{
	namespace common
	{
		struct PhysicMaterial final
			: public vis::Aggregate<PhysicMaterial, sta::ADynamicType>
		{
		public:
			explicit PhysicMaterial()
			{}

			// Methods
			friend class vis::Aggregate<PhysicMaterial>;
			template <typename VisitorType, typename ThisType>
			// ReSharper disable once CppMemberFunctionMayBeStatic
			static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
			{
				a_visitor.visit(vis::nvp("Restitution", a_this.m_restitution));
				a_visitor.visit(vis::nvp("Friction", a_this.m_friction));
				a_visitor.visit(vis::nvp("RollingFriction", a_this.m_rollingFriction));
				a_visitor.visit(vis::nvp("SpinningFriction", a_this.m_spinningFriction));
				a_visitor.visit(vis::nvp("ContactStiffness", a_this.m_contactStiffness));
				a_visitor.visit(vis::nvp("ContactDamping", a_this.m_contactDamping));
			}

			btScalar m_restitution{ 0.0f };
			btScalar m_friction{ 0.5f };
			btScalar m_rollingFriction{ 0.0f };
			btScalar m_spinningFriction{ 0.0f };
			btScalar m_contactStiffness{ 9.99999984e+17f };
			btScalar m_contactDamping{ 0.1f };
		};
	}
}
