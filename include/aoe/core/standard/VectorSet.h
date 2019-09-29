#pragma once

#include <algorithm>
#include <vector>

#include <aoe/core/standard/Allocator.h>

namespace aoe
{
	namespace sta
	{
		template <typename KeyType, typename EqualType = std::equal_to<>
			, typename AllocatorType = std::allocator<KeyType>>
		class VectorSet
			: public std::vector<KeyType, AllocatorType>
		{
			// Aliases
			using Base = std::vector<KeyType, AllocatorType>;
		public:
			// Constructors
			VectorSet() = default;

			explicit VectorSet(AllocatorType const& a_allocator)
				: Base{ a_allocator }
			{}

			// Methods
			decltype(auto) begin() const
			{
				return Base::begin();
			}

			decltype(auto) end() const
			{
				return Base::end();
			}

			decltype(auto) find(KeyType const& a_key)
			{
				return std::find_if(begin(), end(), [&a_key](auto const& a_item)
				{
					return EqualType{}(a_item, a_key);
				});
			}

			decltype(auto) find(KeyType const& a_key) const
			{
				return std::find_if(begin(), end(), [&a_key](auto const& a_item)
				{
					return EqualType{}(a_item, a_key);
				});
			}

			decltype(auto) emplace(KeyType&& a_key)
			{
				auto t_it = find(a_key);
				if (t_it != end())
				{
					return std::make_pair(t_it, false);
				}

				Base::emplace_back(std::move(a_key));
				t_it = end();
				--t_it;
				return std::make_pair(t_it, true);
			}
		};

		namespace pmr
		{
			template <typename KeyType, typename EqualType = std::equal_to<>>
			using VectorSet = sta::VectorSet<KeyType, EqualType
				, Allocator<KeyType>>;
		}
	}
}