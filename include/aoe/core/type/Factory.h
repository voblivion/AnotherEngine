#pragma once

#include <utility>
#include <typeindex>

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

		template <typename LeftType, typename RightType>
		struct Factory<std::pair<LeftType, RightType>>
		{
			std::pair<LeftType, RightType> operator()() const
			{
				return std::make_pair(Factory<LeftType>{}(), Factory<RightType>{}());
			}
		};

		template <>
		struct Factory<std::type_index>
		{
			std::type_index operator()() const
			{
				return typeid(void);
			}
		};
	}
}