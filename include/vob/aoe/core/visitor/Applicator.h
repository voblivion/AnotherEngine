#pragma once

#include <vob/aoe/core/type/Applicator.h>

namespace vob::aoe::vis
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

	template <typename PolymorphicBaseType, typename VisitorType>
	using Applicator = type::Applicator<
		PolymorphicBaseType
		, ApplyVisitorHolder<VisitorType>::template Type
		, VisitorType&
	>;
}