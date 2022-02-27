#pragma once

#include <vob/aoe/common/data/filesystem/FileSystemIndexer.h>

#include <vob/misc/type/factory.h>


namespace vob::aoe::common
{
	struct FileSystemVisitorContext
	{
		FileSystemVisitorContext(
			FileSystemIndexer& a_fileSystemIndexer
			, std::filesystem::path const& a_loadingDataPath
			, data::ADatabase& a_database
		)
			: m_fileSystemIndexer{ a_fileSystemIndexer }
			, m_loadingDataPath{ a_loadingDataPath }
			, m_database{ a_database }
		{}

		FileSystemIndexer& m_fileSystemIndexer;
		std::filesystem::path const& m_loadingDataPath;
		data::ADatabase& m_database;
	};
}
