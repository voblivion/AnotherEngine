#pragma once

#include <cinttypes>
#include <limits>

namespace vob::aoecs
{
	using entity_id = std::uint64_t;
	constexpr entity_id k_invalid_entity_id = std::numeric_limits<entity_id>::max();
}
