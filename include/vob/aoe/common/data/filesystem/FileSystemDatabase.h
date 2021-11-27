#pragma once

#include <cassert>

#include <vob/aoe/core/data/ADatabase.h>

#include <vob/aoe/common/data/filesystem/MultiFileSystemLoader.h>
#include <vob/aoe/core/data/Cache.h>

namespace vob::aoe::common
{
	class FileSystemDatabase final
		: public data::ADatabase
	{
	public:
		// Constructor
		explicit FileSystemDatabase(
			misty::pmr::registry& a_typeRegistry
			, FileSystemIndexer& a_indexer
		)
			: ADatabase{ a_typeRegistry }
			, m_loader{ a_indexer }
		{}

		MultiFileSystemLoader& getMultiFileSystemLoader()
		{
			return m_loader;
		}

	protected:
		// Methods
		std::shared_ptr<type::ADynamicType> findDynamic(data::Id const a_id) final override
		{
			auto t_data = m_cache.find(a_id);
			if (t_data == nullptr)
			{
				t_data = loadAndCache(a_id);
			}
			return t_data;
		}

	private:
		// Attributes
		mutable data::Cache m_cache;
		MultiFileSystemLoader m_loader;

		// Methods
		std::shared_ptr<type::ADynamicType> loadAndCache(data::Id const a_id)
		{
			auto const t_data = m_loader.load(a_id);
			if (t_data != nullptr)
			{
				m_cache.set(a_id, t_data);
			}
			return t_data;
		}
	};
}
