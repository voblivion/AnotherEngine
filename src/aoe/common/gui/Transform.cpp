#include <vob/aoe/common/gui/Transform.h>

namespace vob::aoe::common::gui
{
	Transform::Transform(type::CloneCopier const& a_cloneCopier)
		: m_x{ a_cloneCopier }
		, m_y{ a_cloneCopier }
		, m_width{ a_cloneCopier }
		, m_height{ a_cloneCopier }
	{}

	void Transform::apply(Constraint& a_constraint, Vector2 const& a_viewSize) const
	{
		auto const tryApply = [&a_viewSize](auto const& a_rule, auto& a_value)
		{
			if (a_rule != nullptr)
			{
				a_rule->tryApply(a_value, a_viewSize);
			}
		};

		tryApply(m_x, a_constraint.m_x);
		tryApply(m_y, a_constraint.m_y);
		tryApply(m_width, a_constraint.m_width);
		tryApply(m_height, a_constraint.m_height);
	}
}
