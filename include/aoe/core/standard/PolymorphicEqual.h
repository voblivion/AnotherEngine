#pragma once

#include <cstdlib>
#include <typeindex>

namespace aoe
{
	namespace sta
	{
		template <typename PointerType>
		struct DefaultReferenceGetter
		{
			auto& operator()(PointerType const& a_pointer) const
			{
				return *a_pointer;
			}
		};

		template <typename PointerType, typename ReferenceGetter
			= DefaultReferenceGetter<PointerType>>
		struct PolymorphicEqual
		{
			constexpr auto operator()(
				PointerType const& a_lhs, PointerType const& a_rhs) const
			{
				return typeid(ReferenceGetter{}(a_lhs))
					== typeid(ReferenceGetter{}(a_rhs));
			}
		};
	}
}