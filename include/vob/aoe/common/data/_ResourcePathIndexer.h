#pragma once
#include <cassert>
#include <optional>
#include <unordered_map>

#include <vob/aoe/core/data/Id.h>
#include <vob/aoe/common/data/ResourcePath.h>

namespace vob::aoe::common
{
	/* TODO deprecate :
	 * 
	 * FileIndexer.h
	 */

	class ResourcePathIndexer
	{
	public:
		// Aliases
		using IndexToPathMap = std::unordered_map<data::Id, ResourcePath>;
		using PathToIndexMap = std::unordered_map<ResourcePath, data::Id>;

		// Constructors
		explicit ResourcePathIndexer(AllocatorType const& a_allocator = {})
			: m_pathToIndexMap{ a_allocator }
			, m_indexToPathMap{ a_allocator }
		{}

		explicit ResourcePathIndexer(IndexToPathMap a_indexToPathMap)
			: m_pathToIndexMap{ a_indexToPathMap.get_allocator() }
			, m_indexToPathMap{ std::move(a_indexToPathMap) }
		{
			for (auto const& indexToPathPair : m_indexToPathMap)
			{
				m_pathToIndexMap.emplace(indexToPathPair.second, indexToPathPair.first);
				m_nextDataId = std::max(m_nextDataId, indexToPathPair.first + 1);
			}
		}

		// Methods
		template <typename Visitor>
		void accept(Visitor& a_visitor)
		{
			a_visitor.visit(m_indexToPathMap);
			for (auto const& indexToPathPair : m_indexToPathMap)
			{
				m_pathToIndexMap.emplace(indexToPathPair.second, indexToPathPair.first);
			}
		}

		ResourcePath const* findResourcePath(data::Id const a_dataId) const
		{
			auto const indexToPathIt = m_indexToPathMap.find(a_dataId);
			if (indexToPathIt != m_indexToPathMap.end())
			{
				return &(indexToPathIt->second);
			}

			return nullptr;
		}

		std::optional<data::Id> findDataId(ResourcePath const& a_resourcePath)
		{
			auto const pathToIndexIt = m_pathToIndexMap.find(a_resourcePath);
			if (pathToIndexIt != m_pathToIndexMap.end())
			{
				return pathToIndexIt->second;
			}

			return {};
		}

		data::Id getDataId(ResourcePath const& a_resourcePath)
		{
			if (const auto existingId = findDataId(a_resourcePath))
			{
				return existingId.value();
			}

			return createDataId(a_resourcePath);
		}

		void insert(ResourcePath const& a_resourcePath, data::Id a_dataId)
		{
			assert(a_dataId >= m_nextDataId);
			assert(m_pathToIndexMap.find(a_resourcePath) == m_pathToIndexMap.end());
			set(a_resourcePath, a_dataId);
			m_nextDataId = a_dataId + 1;
		}

	private:
		// Attributes
		data::Id m_nextDataId = 0;
		PathToIndexMap m_pathToIndexMap;
		IndexToPathMap m_indexToPathMap;

		// Methods
		data::Id createDataId(ResourcePath const& a_resourcePath)
		{
			auto const dataId = m_nextDataId;
			insert(a_resourcePath, dataId);
			return dataId;
		}

		void set(ResourcePath const& a_resourcePath, data::Id a_dataId)
		{
			m_pathToIndexMap.emplace(a_resourcePath, a_dataId);
			m_indexToPathMap.emplace(a_dataId, a_resourcePath);
		}
	};
}
