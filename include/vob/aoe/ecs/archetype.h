#pragma once

#include <algorithm>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>


namespace vob::aoecs
{
	class archetype
	{
	public:
		archetype() = default;

		template <typename... TArgs>
		explicit archetype(std::unordered_set<std::type_index, TArgs...> const& a_componentTypes)
			: m_componentTypes{ a_componentTypes }
		{
		}

		template <typename... TArgs>
		explicit archetype(std::unordered_map<std::type_index, TArgs...> const& a_componentMap)
		{
			m_componentTypes.reserve(a_componentMap.size());
			for (auto const& componentEntry : a_componentMap)
			{
				m_componentTypes.emplace(componentEntry.first);
			}
		}

		template <typename TComponent>
		void add()
		{
			m_componentTypes.emplace(typeid(TComponent));
		}

		auto size() const
		{
			return m_componentTypes.size();
		}

		auto begin() const
		{
			return m_componentTypes.begin();
		}

		auto end() const
		{
			return m_componentTypes.end();
		}

		bool contains(std::type_index const& a_type) const
		{
			return m_componentTypes.contains(a_type);
		}

		friend bool operator<=(archetype const& a_lhs, archetype const& a_rhs)
		{
			return std::all_of(
				a_lhs.begin(),
				a_lhs.end(),
				[&a_rhs](auto const& a_type) { return a_rhs.contains(a_type); }
			);
		}

		friend bool operator==(archetype const& a_lhs, archetype const& a_rhs)
		{
			return a_lhs.size() == a_rhs.size() && a_lhs <= a_rhs;
		}

	private:
		std::pmr::unordered_set<std::type_index> m_componentTypes;
	};
}

namespace std
{
	template <>
	struct hash<vob::aoecs::archetype>
	{
		size_t operator()(vob::aoecs::archetype const& a_archetype) const
		{
			auto acc = std::size_t{ 0 };
			for (auto const& type : a_archetype)
			{
				acc ^= std::hash<std::type_index>{}(type);
			}
			return acc;
		}
	};
}
