#pragma once

#include <vob/aoe/core/visitor/Aggregate.h>

#include <vob/aoe/common/gui/ARule.h>

namespace vob::aoe::common::gui
{
	struct AbsoluteRule final
		: public vis::Aggregate<AbsoluteRule, ARule>
	{
		// Methods
		friend class vis::Aggregate<AbsoluteRule, ARule>;
		template <typename VisitorType, typename ThisType>
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
			a_visitor.visit(a_this.m_value);
		}

		float value(Vector2 const& a_viewSize) const override
		{
			return m_value;
		}

		float m_value{ 100.0f };
	};
}