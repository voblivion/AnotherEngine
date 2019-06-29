#pragma once

#include <cinttypes>

namespace aoe
{
	namespace ecs
	{
		using EntityId = std::uint64_t;
		EntityId const g_invalidEntityId = ULLONG_MAX;
	}
}