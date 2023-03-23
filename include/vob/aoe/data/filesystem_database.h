#pragma once

#include <vob/aoe/data/cache.h>
#include <vob/aoe/data/database.h>
#include <vob/aoe/data/filesystem_indexer.h>

#include <vob/misc/std/filesystem.h>

#include <mutex>
#include <unordered_map>


namespace vob::aoedt
{
	template <typename TLoader>
	struct file_loader_traits
	{
		using type = decltype(
			std::declval<TLoader const>().load(std::declval<std::filesystem::path const&>()));
	};

	template <typename TLoader>
	class filesystem_database final
		: public database<typename file_loader_traits<TLoader>::type>
	{
		using data_type = typename file_loader_traits<TLoader>::type;
	public:
		explicit filesystem_database(filesystem_indexer const& a_indexer, TLoader&& a_loader)
			: m_indexer{ a_indexer }
			, m_loader{ std::move(a_loader) }
		{}

		template <typename... TArgs>
		explicit filesystem_database(filesystem_indexer const& a_indexer, TArgs&&... a_args)
			: m_indexer{ a_indexer }
			, m_loader{ std::forward<TArgs>(a_args)... }
		{
		}

		std::shared_ptr<data_type const> find(id const a_id) const final override
		{
			auto const lock = std::lock_guard{ m_mutex };
			auto const cached = m_cache.find_or_erase(a_id);
			if (cached != nullptr)
			{
				return cached;
			}

			return load_and_cache_unsafe(a_id);
		}

		auto const& get_indexer()
		{
			return m_indexer;
		}

		auto const& get_loader()
		{
			return m_loader;
		}

	private:
		filesystem_indexer const& m_indexer;
		TLoader m_loader;
		mutable std::mutex m_mutex;
		mutable cache<data_type> m_cache;

		std::shared_ptr<data_type> load_and_cache_unsafe(id const a_id) const
		{
			auto const data = load_unsafe(a_id);
			m_cache.update(a_id, data);
			return data;
		}

		std::shared_ptr<data_type> load_unsafe(id const a_id) const
		{
			auto const filePath = m_indexer.find_path(a_id);
			if (filePath == nullptr)
			{
				return nullptr;
			}

			// TODO allocate_shared
			return std::make_shared<data_type>(m_loader.load(*filePath));
		}
	};
}
