#pragma once

#include <vob/aoe/common/data/filesystem/FileSystemIndexer.h>

#include <vob/misc/type/factory.h>


namespace vob::aoe::common
{
	struct FileSystemVisitorContext
	{
		FileSystemVisitorContext(
			misty::pmr::factory const& a_factory
			, FileSystemIndexer& a_fileSystemIndexer
			, std::filesystem::path const& a_loadingDataPath
			, data::ADatabase& a_database
		)
			: m_factory{ a_factory }
			, m_fileSystemIndexer{ a_fileSystemIndexer }
			, m_loadingDataPath{ a_loadingDataPath }
			, m_database{ a_database }
		{}

		auto const& get_factory() const
		{
			return m_factory;
		}

		auto const& get_base_path() const
		{
			return m_loadingDataPath;
		}

		misty::pmr::factory const& m_factory;
		FileSystemIndexer& m_fileSystemIndexer;
		std::filesystem::path const& m_loadingDataPath;
		data::ADatabase& m_database;
	};
}
