#pragma once

#include <vob/aoe/debug/DebugNameComponent.h>

#include <entt/entt.hpp>


namespace vob::aoedb
{
	using DebugNameEntities = decltype(std::declval<entt::registry>().view<DebugNameComponent const>());
	// TODO: ideally it would return string_view but ImGui doesn't support it nicely, maybe zstring_view in the future at least?
	inline char const* getImmediateUseDebugNameCStr(DebugNameEntities const& a_debugNameEntities, entt::entity a_entity)
	{
		if (a_entity == entt::null)
		{
			return "<null>";
		}

		if (a_debugNameEntities.contains(a_entity))
		{
			return std::get<DebugNameComponent const&>(a_debugNameEntities.get(a_entity)).value.c_str();
		}

		static thread_local char debugName[21];
		std::snprintf(debugName, sizeof(debugName), "%lld", static_cast<uint64_t>(a_entity));
		return debugName;
	}
}
