#pragma once

#include <utility>

namespace aoe
{
	namespace type
	{
		template <typename ValueType>
		struct Factory
		{
			ValueType operator()() const
			{
				return {};
			}
		};
	}
}