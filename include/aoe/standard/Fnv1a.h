#pragma once

#include <cstddef>
#include <cinttypes>

namespace aoe
{
	namespace sta
	{
		inline constexpr std::uint64_t fnv1a(char const* a_str
			, std::size_t const a_size
			, std::uint64_t const a_value = 0xcbf29ce484222325)
		{
			return a_size == 0 ? a_value
				: fnv1a(&a_str[1], a_size - 1
					, (a_value ^ std::uint64_t(a_str[0])) * 0x100000001b3);
		}

		namespace literals
		{
			inline std::uint64_t operator "" _id(char const* a_str
				, std::size_t const a_size)
			{
				return fnv1a(a_str, a_size);
			}
		}
	}
}