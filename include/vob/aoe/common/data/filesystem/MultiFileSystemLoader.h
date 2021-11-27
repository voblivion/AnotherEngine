#pragma once

#include <vob/misc/std/ignorable_assert.h>

#include <vob/aoe/common/data/filesystem/AFileSystemLoader.h>
#include <vob/aoe/common/data/filesystem/FileSystemIndexer.h>
#include <vob/aoe/core/data/Id.h>
#include <SFML/System/InputStream.hpp>

namespace vob::aoe::common
{
	class MultiFileSystemLoader
	{
	public:
		// Constructors
		explicit MultiFileSystemLoader(FileSystemIndexer& a_indexer)
			: m_indexer{ a_indexer }
		{}

		// Methods
		void registerLoader(AFileSystemLoader& a_dataLoader)
		{
			m_loaders.emplace_back(a_dataLoader);
		}

		std::shared_ptr<type::ADynamicType> load(data::Id const a_id)
		{
			if (auto const cleanPath = m_indexer.findPath(a_id))
			{
				return load(*cleanPath);
			}
			// ignorable_assert(false);
			return nullptr;
		}

		auto& getIndexer()
		{
			return m_indexer;
		}

	private:
		// Attributes
		FileSystemIndexer& m_indexer;
		std::vector<std::reference_wrapper<AFileSystemLoader>> m_loaders;

		// Methods
		AFileSystemLoader* findLoader(std::filesystem::path const& a_path)
		{
			auto const it = std::find_if(
				m_loaders.begin()
				, m_loaders.end()
				, [&a_path](auto const& a_loaderEntry)
			{
				return a_loaderEntry.get().canLoad(a_path);
			});

			return it != m_loaders.end() ? &(it->get()) : nullptr;
		}

		std::shared_ptr<type::ADynamicType> load(std::filesystem::path const& a_path)
		{
			if (auto const loader = findLoader(a_path))
			{
				return loader->load(a_path);
			}
			ignorable_assert(false);
			return nullptr;
		}
	};
}
