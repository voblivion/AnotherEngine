#pragma once

#include <algorithm>
#include <vector>

#include <aoe/core/standard/Allocator.h>
#include <aoe/core/standard/Utility.h>

namespace aoe
{
	namespace sta
	{
		template <typename KeyType, typename ValueType, typename AllocatorType
			= std::allocator<std::pair<KeyType const, ValueType>>>
			class VectorMap
			: public std::vector<std::pair<KeyType const, ValueType>, AllocatorType>
		{
			// Aliases
			using Base = std::vector<std::pair<KeyType const, ValueType>
				, AllocatorType>;
		public:
			// Constructors
			VectorMap() = default;

			explicit VectorMap(AllocatorType const& a_allocator)
				: Base{ a_allocator }
			{}

			// Methods
			using Base::begin;
			using Base::end;

			decltype(auto) find(KeyType const& a_key)
			{
				return std::find_if(begin(), end(), [&a_key](auto const& a_pair)
				{
					return a_pair.first == a_key;
				});
			}

			decltype(auto) find(KeyType const& a_key) const
			{
				return std::find_if(begin(), end(), [&a_key](auto const& a_pair)
				{
					return a_pair.first == a_key;
				});
			}

			decltype(auto) emplace(std::pair<KeyType const, ValueType>&& a_pair)
			{
				auto t_it = find(a_pair.first);
				if (t_it != end())
				{
					return std::make_pair(t_it, false);
				}

				Base::emplace_back(std::move(a_pair));
				t_it = end();
				--t_it;
				return std::make_pair(t_it, true);
			}

			decltype(auto) emplace(KeyType a_key, ValueType a_value = {})
			{
				return emplace(std::make_pair(std::move(a_key)
					, std::move(a_value)));
			}

			// Operators
			decltype(auto) operator[](KeyType const& a_key)
			{
				auto t_res = emplace(a_key);
				return t_res.first;
			}
		};

		namespace pmr
		{
			template <typename KeyType, typename ValueType>
			using VectorMap = sta::VectorMap<KeyType, ValueType
				, Allocator<std::pair<KeyType const
				, ValueType>>>;
		}
	}
}