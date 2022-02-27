#pragma once

#include <LinearMath/btScalar.h>
#include <vob/misc/visitor/name_value_pair.h>


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

namespace vob::misvi
{
	template <typename VisitorType, typename ThisType>
	requires std::is_same_v<std::remove_cvref_t<ThisType>, aoe::common::PhysicMaterial>
	bool accept(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(misvi::nvp("Restitution", a_this.m_restitution));
		a_visitor.visit(misvi::nvp("Friction", a_this.m_friction));
		a_visitor.visit(misvi::nvp("RollingFriction", a_this.m_rollingFriction));
		a_visitor.visit(misvi::nvp("SpinningFriction", a_this.m_spinningFriction));
		a_visitor.visit(misvi::nvp("ContactStiffness", a_this.m_contactStiffness));
		a_visitor.visit(misvi::nvp("ContactDamping", a_this.m_contactDamping));
		return true;
	}
}
