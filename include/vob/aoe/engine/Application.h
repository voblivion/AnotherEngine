#pragma once

#include <vob/aoe/debug/Check.h>

#include "entt/entt.hpp"

#include <type_traits>
#include <utility>


namespace vob::aoeng
{
	class Application
	{
	public:
		template <typename TValue>
		void add(TValue&& a_value)
		{
			m_entries.insert_or_assign(
				entt::type_hash<std::remove_cvref_t<TValue>>::value(), entt::forward_as_any(std::forward<TValue>(a_value)));
		}

		template <typename TValue>
		TValue& get()
		{
			auto const entryIt = m_entries.find(entt::type_hash<TValue>::value());
			VOB_AOE_CHECK_TERMINATE(
				entryIt != m_entries.end(), "Application is missing a {}.", entt::type_name<TValue>::value());
			return entt::any_cast<TValue&>(entryIt->second);
		}

	private:
		entt::dense_map<entt::id_type, entt::any> m_entries;
	};
}
