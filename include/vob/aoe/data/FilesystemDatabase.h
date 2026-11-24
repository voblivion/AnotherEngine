#pragma once

#include <vob/aoe/data/Cache.h>
#include <vob/aoe/data/Database.h>
#include <vob/aoe/data/FilesystemIndexer.h>

#include <mutex>
#include <unordered_map>


namespace vob::aoedt
{
	template <typename TLoader>
	struct FileLoaderTraits
	{
		using Type = decltype(
			std::declval<TLoader const>().load(std::declval<std::filesystem::path const&>()));
	};

	template <typename TLoader>
	class FilesystemDatabase final
		: public Database<typename FileLoaderTraits<TLoader>::Type>
	{
		using DataType = typename FileLoaderTraits<TLoader>::Type;
	public:
		explicit FilesystemDatabase(FilesystemIndexer const& a_indexer, TLoader&& a_loader)
			: m_indexer{ a_indexer }
			, m_loader{ std::move(a_loader) }
		{}

		template <typename... TArgs>
		explicit FilesystemDatabase(FilesystemIndexer const& a_indexer, TArgs&&... a_args)
			: m_indexer{ a_indexer }
			, m_loader{ std::forward<TArgs>(a_args)... }
		{
		}

		std::shared_ptr<DataType const> find(Id const a_id) const final override
		{
			auto const lock = std::lock_guard{ m_mutex };
			auto const cached = m_cache.findOrErase(a_id);
			if (cached != nullptr)
			{
				return cached;
			}

			return loadAndCacheUnsafe(a_id);
		}

		auto const& getIndexer()
		{
			return m_indexer;
		}

		auto const& getLoader()
		{
			return m_loader;
		}

	private:
		FilesystemIndexer const& m_indexer;
		TLoader m_loader;
		mutable std::mutex m_mutex;
		mutable Cache<DataType> m_cache;

		std::shared_ptr<DataType> loadAndCacheUnsafe(Id const a_id) const
		{
			auto const data = loadUnsafe(a_id);
			m_cache.update(a_id, data);
			return data;
		}

		std::shared_ptr<DataType> loadUnsafe(Id const a_id) const
		{
			auto const filePath = m_indexer.findPath(a_id);
			if (filePath == nullptr)
			{
				return nullptr;
			}

			// TODO allocate_shared
			return std::make_shared<DataType>(m_loader.load(m_indexer.getDataPath() / *filePath));
		}
	};
}
