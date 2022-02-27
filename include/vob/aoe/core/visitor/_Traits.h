#pragma once

#include <type_traits>


namespace vob::aoe::vis
{
    template <typename SourceType, typename TargetType>
    using sameConst = std::conditional_t<std::is_const_v<SourceType>, TargetType const, TargetType>;

	template <typename Type, typename BaseType>
	class Visitable : public BaseType
	{
	public:
		template <typename VisitorType>
		void accept(VisitorType& a_visitor)
		{
			Type::accept(a_visitor, static_cast<Type&>(*this));
		}

		template <typename VisitorType>
		void accept(VisitorType& a_visitor) const
		{
			Type::accept(a_visitor, static_cast<Type const&>(*this));
		}
	};

	template <typename VisitorType, typename ValueType>
	concept StaticAcceptVisitable = requires(ValueType& a_value)
	{
		{ ValueType::accept(std::declval<VisitorType&>(), a_value) };
	};

	template <typename VisitorType, typename ValueType>
	concept FreeAcceptVisitable = requires(VisitorType& a_visitor, ValueType& a_value)
	{
		{ accept(a_visitor, a_value) };
	};

    template <typename VisitorType, typename ValueType>
    concept MemberAcceptVisitable = requires(VisitorType& a_visitor, ValueType& a_value)
    {
        { a_value.accept(a_visitor) };
    };

	template <typename Type, typename ThisType>
	concept IsSelfVisit = std::is_same_v<std::remove_cvref_t<Type>, ThisType>;

	// TODO: remove
	template <typename Type, typename ValueType>
	using visitIfType = std::enable_if_t<std::is_same_v<Type, std::remove_const_t<ValueType>>>;

	template <typename BaseType, typename ValueType>
	using visitIfBaseType = std::enable_if_t<std::is_base_of_v<BaseType, std::remove_const_t<ValueType>>>;
}
