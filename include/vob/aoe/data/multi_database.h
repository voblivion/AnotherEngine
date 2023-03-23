#pragma once

#include <vob/aoe/data/database.h>

#include <typeindex>
#include <unordered_map>


namespace vob::aoedt
{
	namespace detail
	{
		struct database_holder_base
		{
			virtual ~database_holder_base() = default;
		};

		template <typename TData>
		struct database_holder
			: public database_holder_base
		{
			explicit database_holder(database<TData> const& a_database)
				: m_database{ a_database }
			{}

			database<TData> const& m_database;
		};
	}

	class multi_database
	{
	public:

		template <typename TData>
		void register_database(database<TData> const& a_database)
		{
			m_databases.try_emplace(
				typeid(TData)
				, std::make_shared<detail::database_holder<TData>>(a_database));
		}

		template <typename TData>
		std::shared_ptr<TData const> find(id const& a_id) const
		{
			auto const it = m_databases.find(typeid(TData));
			if (it == m_databases.end())
			{
				return nullptr;
			}

			return static_cast<detail::database_holder<TData>&>(*it->second).m_database.find(a_id);
		}

	private:
		std::pmr::unordered_map<std::type_index, std::shared_ptr<detail::database_holder_base>>
			m_databases;
	};
}
