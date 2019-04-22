#pragma once

#include <memory_resource>

namespace aoe
{
	namespace sta
	{
		template <typename Allocator, typename Type>
		using RebindAlloc = typename std::allocator_traits<Allocator>
			::template rebind_alloc<Type>;

		template <typename Type>
		using Allocator = std::pmr::polymorphic_allocator<Type>;
	}
}