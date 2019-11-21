#pragma once

#include <vob/aoe/core/type/Clone.h>
#include <vob/aoe/core/visitor/Aggregate.h>

#include <vob/aoe/api.h>
#include <vob/aoe/common/gui/ARule.h>
#include <vob/aoe/common/gui/Constraint.h>
#include <vob/aoe/common/space/Vector.h>


namespace vob::aoe::common::gui
{
	struct Transform
		: public vis::Aggregate<Transform>
	{
		explicit VOB_AOE_API Transform(type::CloneCopier const& a_cloneCopier);

		// Methods
		friend class vis::Aggregate<Transform>;
		template <typename VisitorType, typename ThisType>
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this);

		void VOB_AOE_API apply(
			Constraint& a_constraint
			, Vector2 const& a_viewSize
		) const;

		type::Clone<ARule> m_x;
		type::Clone<ARule> m_y;
		type::Clone<ARule> m_width;
		type::Clone<ARule> m_height;
	};

	template <typename VisitorType, typename ThisType>
	void Transform::makeVisit(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(vis::nvp("X", a_this.m_x));
		a_visitor.visit(vis::nvp("Y", a_this.m_y));
		a_visitor.visit(vis::nvp("Width", a_this.m_width));
		a_visitor.visit(vis::nvp("Height", a_this.m_height));
	}

}
