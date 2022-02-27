#pragma once

#include <vob/misc/type/applicator.h>

namespace vob::
{
	template <typename VisitorType>
	struct ApplyVisitorHolder
	{
		template <typename ValueType>
		struct Type
		{
			void operator()(
				ValueType& a_value
				, VisitorType& a_visitor
			) const
			{
 				a_visitor.visit(a_value);
			}
		};
	};

	template <typename VisitorType>
	using ApplyVisitor = typename ApplyVisitorHolder<VisitorType>::Type;

	template <bool t_const, typename VisitorType>
	using Applicator = misty::pmr::applicator<
		ApplyVisitorHolder<VisitorType>::template Type
		, t_const
		, VisitorType&
	>;
}