#pragma once

#include <vob/sta/ignorable_assert.h>

#include <vob/aoe/core/data/ALoader.h>
#include <vob/aoe/core/data/Id.h>

namespace vob::aoe::data
{
	template <typename StreamProviderType>
	class DatabaseLoader
	{
	public:
		// Aliases
		using InputStream = typename StreamProviderType::InputStream;

		// Constructors
		explicit DatabaseLoader(StreamProviderType a_streamProvider)
			: m_streamProvider{ a_streamProvider }
		{}

		// Methods
		void registerLoader(ALoader& a_dataLoader)
		{
			m_loaders.emplace_back(a_dataLoader);
		}

		std::shared_ptr<type::ADynamicType> load(Id a_dataId)
		{
			// TODO : lock stream for multi-threaded loading ?
			InputStream inputStream;
			if (m_streamProvider.find(a_dataId, inputStream))
			{
				return load(a_dataId, inputStream);
			}
			ignorable_assert(false);
			return nullptr;
		}

	private:
		// Attributes
		StreamProviderType m_streamProvider;
		std::vector<std::reference_wrapper<ALoader>> m_loaders;

		// Methods
		ALoader* findInputStreamLoader(InputStream& a_inputStream)
		{
			auto it = std::find_if(
				m_loaders.begin()
				, m_loaders.end()
				, [&a_inputStream](auto const& a_loaderEntry)
			{
				return a_loaderEntry.get().canLoad(a_inputStream);
			});

			return it != m_loaders.end() ? &(it->get()) : nullptr;
		}

		std::shared_ptr<type::ADynamicType> load(Id a_dataId, InputStream& a_inputStream)
		{
			auto loader = findInputStreamLoader(a_inputStream);
			ignorable_assert(loader != nullptr);
			if (loader)
			{
				return loader->load(a_dataId, a_inputStream);
			}
			return nullptr;
		}
	};
}