#pragma once

#include <type_traits>

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ValueType>
	class HasMemberAccept
	{
	private:
		template <typename Vis, typename Val>
		static auto test(int)
			-> decltype(
				std::declval<Val>().accept(
					std::declval<Vis&>()
				), std::true_type{}
			)
		{
			return std::true_type{};
		}

		template <typename, typename>
		static std::false_type test(...) { return std::false_type{}; }

	public:
		static constexpr bool value = decltype(
			test<VisitorType, ValueType>(0))::value;
	};

	template <typename VisitorType, typename ValueType>
	constexpr bool hasMemberAccept = HasMemberAccept<VisitorType, ValueType>::value;

	template <typename VisitorType, typename ValueType>
	class HasNonMemberAccept
	{
	private:
		template <typename Vis, typename Val>
		static auto test(int)
			-> decltype(accept(std::declval<Vis&>(), std::declval<Val>()), std::true_type{})
		{
			return std::true_type{};
		}

		template <typename, typename>
		static std::false_type test(...) { return std::false_type{}; }

	public:
		static constexpr bool value = decltype(test<VisitorType, ValueType>(0))::value;
	};

	template <typename VisitorType, typename ValueType>
	constexpr bool hasNonMemberAccept = HasNonMemberAccept<VisitorType, ValueType>::value;
}
