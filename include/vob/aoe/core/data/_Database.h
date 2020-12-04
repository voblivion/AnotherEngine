#pragma once
#include <cassert>
#include <memory>


#include <vob/aoe/core/data/ADatabase.h>
#include <vob/aoe/core/data/Cache.h>
#include <vob/aoe/core/data/Id.h>


namespace vob::aoe::data
{
	template <typename DatabaseLoaderType>
	class Database final
		: public ADatabase
	{
	public:
		// Aliases
		using DatabaseLoader = DatabaseLoaderType;

		// Constructors
		explicit Database(
			type::TypeRegistry& a_typeRegistry
			, DatabaseLoader&& a_databaseLoader
		)
			: ADatabase{ a_typeRegistry }
			, m_cache{ a_allocator }
			, m_loader{ std::forward<DatabaseLoader>(a_databaseLoader) }
		{}

		DatabaseLoader& getLoader()
		{
			return m_loader;
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
		mutable Cache m_cache;
		DatabaseLoader m_loader;

		// Methods
		std::shared_ptr<type::ADynamicType> loadAndCache(Id const a_dataId)
		{
			auto const t_data = m_loader.load(a_dataId);
			if (t_data != nullptr)
			{
				m_cache.set(a_dataId, t_data);
			}
			return t_data;
		}

	};
}
