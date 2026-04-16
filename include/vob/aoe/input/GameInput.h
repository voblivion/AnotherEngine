#pragma once

#include <cstdint>
#include <utility>


namespace vob::aoein
{
	struct GameInputValueId
	{
		int32_t id = -1;

		static friend bool operator==(GameInputValueId const& a_lhs, GameInputValueId const& a_rhs)
		{
			return a_lhs.id == a_rhs.id;
		}
	};


	struct GameInputEventId
	{
		int32_t id = -1;

		static friend bool operator==(GameInputEventId const& a_lhs, GameInputEventId const& a_rhs)
		{
			return a_lhs.id == a_rhs.id;
		}
	};
}

namespace std
{
	template <>
	struct hash<vob::aoein::GameInputValueId>
	{
		size_t operator()(const vob::aoein::GameInputEventId& eventId) const
		{
			return static_cast<size_t>(eventId.id);
		}
	};

	template <>
	struct hash<vob::aoein::GameInputEventId>
	{
		size_t operator()(const vob::aoein::GameInputEventId& eventId) const
		{
			return static_cast<size_t>(eventId.id);
		}
	};
}
