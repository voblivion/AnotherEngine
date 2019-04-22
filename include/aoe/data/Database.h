#pragma once
#include <cassert>
#include <memory>

#include <aoe/data/ADatabase.h>
#include <aoe/data/ALoader.h>
#include <aoe/data/Cache.h>
#include <aoe/data/FormattedInputStream.h>
#include <aoe/data/Id.h>

namespace aoe
{
	namespace data
	{
		template <typename IndexerType>
		class Database final
			: public ADatabase
		{
		public:
			// Aliases
			using AllocatorType = Cache::AllocatorType;

			// Constructors
			explicit Database(sta::TypeRegistry& a_typeRegistry)
				: ADatabase{ a_typeRegistry }
			{}

			explicit Database(sta::TypeRegistry& a_typeRegistry, IndexerType a_indexer)
				: ADatabase{ a_typeRegistry }
				, m_indexer{ std::move(a_indexer) }
			{}

			explicit Database(sta::TypeRegistry& a_typeRegistry, IndexerType a_indexer
				, AllocatorType const& a_allocator)
				: ADatabase{ a_typeRegistry }
				, m_indexer{ a_indexer }
				, m_cache{ a_allocator }
				, m_loaders{ a_allocator }
			{}

			// Methods
			bool hasLoaderFor(FormatId const a_format) const
			{
				return m_loaders.find(a_format) != m_loaders.end();
			}

			void registerLoader(FormatId const a_format, ALoader& a_dataLoader)
			{
				assert(!hasLoaderFor(a_format));
				m_loaders.emplace(a_format, a_dataLoader);
			}

			IndexerType& getIndexer()
			{
				return m_indexer;
			}

		protected:
			// Methods
			std::shared_ptr<ADynamicType> find(Id const a_dataId) final override
			{
				auto t_data = m_cache.find(a_dataId);
				if (t_data == nullptr)
				{
					t_data = loadAndCache(a_dataId);
				}
				return t_data;
			}

		private:
			// Attributes
			IndexerType m_indexer;
			mutable Cache m_cache;
			std::pmr::unordered_map<FormatId
				, std::reference_wrapper<ALoader>> m_loaders;

			// Methods
			std::shared_ptr<ADynamicType> loadAndCache(Id const a_dataId)
			{
				auto const t_data = load(a_dataId);
				if (t_data != nullptr)
				{
					m_cache.set(a_dataId, t_data);
				}
				return t_data;
			}

			std::shared_ptr<ADynamicType> load(Id a_dataId)
			{
				if (auto t_formattedInputStream = m_indexer.find(a_dataId))
				{
					auto const t_it = m_loaders.find(
						t_formattedInputStream->getFormat());
					if (t_it != m_loaders.end())
					{
						return t_it->second.get().load(
							t_formattedInputStream->getInputStream());
					}
				}
				return nullptr;
			}
		};
	}
}
