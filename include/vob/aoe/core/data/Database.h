#pragma once
#include <cassert>
#include <memory>

#include <vob/sta/ignorable_assert.h>

#include <vob/aoe/core/data/ADatabase.h>
#include <vob/aoe/core/data/ALoader.h>
#include <vob/aoe/core/data/Cache.h>
#include <vob/aoe/core/data/FormattedInputStream.h>
#include <vob/aoe/core/data/Id.h>


namespace vob::aoe::data
{
	template <typename StreamProviderType>
	class Database final
		: public ADatabase
	{
	public:
		// Aliases
		using AllocatorType = Cache::AllocatorType;

		// Constructors
		explicit Database(type::TypeRegistry& a_typeRegistry)
			: ADatabase{ a_typeRegistry }
		{}

		explicit Database(type::TypeRegistry& a_typeRegistry
			, StreamProviderType a_streamProvider)
			: ADatabase{ a_typeRegistry }
			, m_streamProvider{ std::move(a_streamProvider) }
		{}

		explicit Database(type::TypeRegistry& a_typeRegistry
			, StreamProviderType a_streamProvider
			, AllocatorType const& a_allocator)
			: ADatabase{ a_typeRegistry }
			, m_streamProvider{ a_streamProvider }
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

		StreamProviderType& getStreamProvider()
		{
			return m_streamProvider;
		}

	protected:
		// Methods
		std::shared_ptr<type::ADynamicType> find(Id const a_dataId) final override
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
		StreamProviderType m_streamProvider;
		mutable Cache m_cache;
		std::pmr::unordered_map<FormatId, std::reference_wrapper<ALoader>> m_loaders;

		// Methods
		std::shared_ptr<type::ADynamicType> loadAndCache(Id const a_dataId)
		{
			auto const t_data = load(a_dataId);
			if (t_data != nullptr)
			{
				m_cache.set(a_dataId, t_data);
			}
			return t_data;
		}

		std::shared_ptr<type::ADynamicType> load(Id a_dataId)
		{
			if (auto t_formattedInputStream = m_streamProvider.find(a_dataId))
			{
				auto const t_it = m_loaders.find(t_formattedInputStream->getFormat());
				ignorable_assert(t_it != m_loaders.end());
				if (t_it != m_loaders.end())
				{
					return t_it->second.get().load(t_formattedInputStream->getInputStream());
				}
			}
			return nullptr;
		}
	};
}
