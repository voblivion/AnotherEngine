#pragma once

#include <vob/aoe/data/Database.h>

#include <typeindex>
#include <unordered_map>


namespace vob::aoedt
{
	namespace detail
	{
		struct DatabaseHolderBase
		{
			virtual ~DatabaseHolderBase() = default;
		};

		template <typename TData>
		struct DatabaseHolder
			: public DatabaseHolderBase
		{
			explicit DatabaseHolder(Database<TData> const& a_database)
				: m_database{ a_database }
			{}

			Database<TData> const& m_database;
		};
	}

	class MultiDatabase
	{
	public:

		template <typename TData>
		void registerDatabase(Database<TData> const& a_database)
		{
			m_databases.try_emplace(
				typeid(TData)
				, std::make_shared<detail::DatabaseHolder<TData>>(a_database));
		}

		template <typename TData>
		std::shared_ptr<TData const> find(Id const& a_id) const
		{
			auto const it = m_databases.find(typeid(TData));
			if (it == m_databases.end())
			{
				return nullptr;
			}

			return static_cast<detail::DatabaseHolder<TData>&>(*it->second).m_database.find(a_id);
		}

	private:
		std::pmr::unordered_map<std::type_index, std::shared_ptr<detail::DatabaseHolderBase>>
			m_databases;
	};
}
