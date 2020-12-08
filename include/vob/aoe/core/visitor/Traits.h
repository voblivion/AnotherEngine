#pragma once

#include <type_traits>

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ValueType>
	class HasAccept
	{
	private:
		template <typename Vis, typename Val>
		static auto test(int)
			-> decltype(accept(std::declval<Vis&>(), std::declval<Val&>()), std::true_type{})
		{
			return std::true_type{};
		}

		template <typename, typename>
		static std::false_type test(...) { return std::false_type{}; }

	public:
		static constexpr bool value = decltype(test<VisitorType, ValueType>(0))::value;
	};

	template <typename VisitorType, typename ValueType>
	constexpr bool hasAcceptValue = HasAccept<VisitorType, ValueType>::value;

	template <typename Type, typename ValueType>
	using visitIfType = std::enable_if_t<std::is_base_of_v<Type, std::remove_const_t<ValueType>>>;
}
