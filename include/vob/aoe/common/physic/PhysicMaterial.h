#pragma once

#include <LinearMath/btScalar.h>
#include <vob/aoe/core/visitor/Utils.h>
#include <vob/aoe/core/visitor/Traits.h>

namespace vob::aoe::common
{
	struct PhysicMaterial final
		: public type::ADynamicType
	{
		btScalar m_restitution{ 0.2f };
		btScalar m_friction{ 0.5f };
		btScalar m_rollingFriction{ 0.3f };
		btScalar m_spinningFriction{ 0.1f };
		btScalar m_contactStiffness{ 9.99999984e+17f };
		btScalar m_contactDamping{ 0.1f };
	};
}

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ThisType>
	visitIfType<common::PhysicMaterial, ThisType> accept(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(vis::nvp("Restitution", a_this.m_restitution));
		a_visitor.visit(vis::nvp("Friction", a_this.m_friction));
		a_visitor.visit(vis::nvp("RollingFriction", a_this.m_rollingFriction));
		a_visitor.visit(vis::nvp("SpinningFriction", a_this.m_spinningFriction));
		a_visitor.visit(vis::nvp("ContactStiffness", a_this.m_contactStiffness));
		a_visitor.visit(vis::nvp("ContactDamping", a_this.m_contactDamping));
	}
}
