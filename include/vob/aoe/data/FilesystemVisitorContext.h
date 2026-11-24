#pragma once

#include <vob/aoe/data/Database.h>
#include <vob/aoe/data/FilesystemIndexer.h>
#include <vob/aoe/data/MultiDatabase.h>

#include <vob/misc/type/factory.h>
#include <vob/misc/visitor/json_reader.h>

#include <filesystem>


namespace vob::aoedt
{
	class FilesystemVisitorContext
	{
	public:
		FilesystemVisitorContext(
			misty::pmr::factory const& a_factory,
			FilesystemIndexer const& a_indexer,
			MultiDatabase const& a_multiDatabase,
			std::filesystem::path a_basePath)
			: m_factory{ a_factory }
			, m_indexer{ a_indexer }
			, m_multiDatabase{ a_multiDatabase }
			, m_basePath{ std::move(a_basePath) }
		{}

		auto const& getFactory() const
		{
			return m_factory;
		}

		auto const& getIndexer() const
		{
			return m_indexer;
		}

		auto const& getMultiDatabase() const
		{
			return m_multiDatabase;
		}

		auto const& getBasePath() const
		{
			return m_basePath;
		}

	private:
		misty::pmr::factory const& m_factory;
		FilesystemIndexer const& m_indexer;
		MultiDatabase const& m_multiDatabase;
		std::filesystem::path m_basePath;
	};

	class FilesystemVisitorContextFactory
	{
	public:
		FilesystemVisitorContextFactory(
			misty::pmr::factory const& a_factory,
			FilesystemIndexer const& a_indexer,
			MultiDatabase const& a_multiDatabase)
			: m_factory{ a_factory }
			, m_indexer{ a_indexer }
			, m_multiDatabase{ a_multiDatabase }
		{}

		auto operator()(std::filesystem::path const& a_filePath) const
		{
			return FilesystemVisitorContext{
				m_factory, m_indexer, m_multiDatabase, a_filePath.lexically_relative(m_indexer.getDataPath()) };
		}

	private:
		misty::pmr::factory const& m_factory;
		FilesystemIndexer const& m_indexer;
		MultiDatabase const& m_multiDatabase;
	};
}

namespace vob::misvi
{
	template <typename... TArgs>
	auto const& get_factory(json_reader<aoedt::FilesystemVisitorContext, TArgs...> const& a_visitor)
	{
		return a_visitor.get_context().getFactory();
	}
}
