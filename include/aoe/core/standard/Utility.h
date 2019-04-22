#pragma once

#include <utility>

namespace std
{
	template <typename Left, typename Right>
	struct hash<pair<Left, Right>>
	{
		size_t operator()(pair<Left, Right> const& a_pair) const
		{
			return hash<Left>{}(a_pair.first) ^ ~(hash<Right>{}(a_pair.second));
		}
	};
}