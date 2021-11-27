#pragma once

#include <cinttypes>

namespace vob::aoe::aoecs
{
	using EntityId = std::uint64_t;
	EntityId const g_invalidEntityId = std::numeric_limits<EntityId>::max();
}
