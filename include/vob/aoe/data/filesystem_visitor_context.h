#pragma once

#include <vob/aoe/data/database.h>
#include <vob/aoe/data/filesystem_indexer.h>
#include <vob/aoe/data/multi_database.h>

#include <vob/misc/type/factory.h>

#include <filesystem>


namespace vob::aoedt
{
	class filesystem_visitor_context
	{
	public:
		filesystem_visitor_context(
			misty::pmr::factory const& a_factory,
			filesystem_indexer const& a_indexer,
			multi_database const& a_multiDatabase,
			std::filesystem::path a_basePath)
			: m_factory{ a_factory }
			, m_indexer{ a_indexer }
			, m_multiDatabase{ a_multiDatabase }
			, m_basePath{ std::move(a_basePath) }
		{}

		auto const& get_factory() const
		{
			return m_factory;
		}

		auto const& get_indexer() const
		{
			return m_indexer;
		}

		auto const& get_multi_database() const
		{
			return m_multiDatabase;
		}

		auto const& get_base_path() const
		{
			return m_basePath;
		}

	private:
		misty::pmr::factory const& m_factory;
		filesystem_indexer const& m_indexer;
		multi_database const& m_multiDatabase;
		std::filesystem::path m_basePath;
	};

	class filesystem_visitor_context_factory
	{
	public:
		filesystem_visitor_context_factory(
			misty::pmr::factory const& a_factory,
			filesystem_indexer const& a_indexer,
			multi_database const& a_multiDatabase)
			: m_factory{ a_factory }
			, m_indexer{ a_indexer }
			, m_multiDatabase{ a_multiDatabase }
		{}

		auto operator()(std::filesystem::path a_basePath) const
		{
			return filesystem_visitor_context{
				m_factory, m_indexer, m_multiDatabase, std::move(a_basePath) };
		}

	private:
		misty::pmr::factory const& m_factory;
		filesystem_indexer const& m_indexer;
		multi_database const& m_multiDatabase;
	};
}
