#pragma once

#include <variant>

namespace vob::aoe::type
{
	template <typename... VariantTypes>
	struct VariantFactory
	{
		template <typename... Types>
		struct VariantIndexFactory;

		template <typename Type>
		struct VariantIndexFactory<Type>
		{
			std::variant<VariantTypes...> operator()(
				std::size_t const a_index) const
			{
				return std::variant<VariantTypes...>{ Type{} };
			}
		};

		template <typename Type, typename... Types>
		struct VariantIndexFactory<Type, Types...>
		{
			std::variant<VariantTypes...> operator()(
				std::size_t const a_index) const
			{
				if (a_index == 0)
				{
					return std::variant<VariantTypes...>{ Type{} };
				}
				else
				{
					return VariantIndexFactory<Types...>{}(a_index - 1);
				}
			}
		};

		std::variant<VariantTypes...> operator()(std::size_t const a_index) const
		{
			return VariantIndexFactory<VariantTypes...>{}(a_index);
		}
	};
}