#pragma once

#include <aoe/core/type/Applicator.h>

namespace aoe
{
	namespace vis
	{
		template <typename VisitorType>
		struct ApplyVisitorHolder
		{
			template <typename ValueType>
			struct Type
			{
				void operator()(ValueType& a_value, VisitorType& a_visitor) const
				{
					a_visitor.visit(a_value);
				}
			};
		};

		template <typename VisitorType>
		using ApplyVisitor = typename ApplyVisitorHolder<VisitorType>::Type;

		template <typename VoidMaybeConst, typename VisitorType>
		using Applicator = type::Applicator<VoidMaybeConst
			, ApplyVisitorHolder<VisitorType>::template Type, VisitorType&>;
	}
}