#pragma once

#include <cassert>
#include <optional>
#include <unordered_map>

#include <vob/aoe/core/data/Id.h>

#include <vob/aoe/common/data/filesystem/FileSystem.h>

namespace vob::aoe::common
{
	class FileSystemIndexer
	{
	public:
		using IndexToPathMap = std::unordered_map<data::Id, std::filesystem::path>;
		using PathToIndexMap = std::unordered_map<PathKey, data::Id>;

		// Constructors
		FileSystemIndexer() = default;

		// Methods
		bool isRegistered(std::filesystem::path const& a_path) const
		{
			auto const pathKey = PathKey{ a_path };
			return m_pathToIndexMap.find(pathKey) != m_pathToIndexMap.end();
		}

		bool isRegistered(data::Id const a_id) const
		{
			return m_indexToPathMap.find(a_id) != m_indexToPathMap.end();
		}

		std::filesystem::path const* findPath(data::Id const a_id) const
		{
			auto const indexToPathIt = m_indexToPathMap.find(a_id);
			if (indexToPathIt != m_indexToPathMap.end())
			{
				return &(indexToPathIt->second);
			}

			return nullptr;
		}

		std::optional<data::Id> findId(std::filesystem::path const& a_path)
		{
			auto const pathKey = PathKey{ a_path };
			auto const pathToIndexIt = m_pathToIndexMap.find(pathKey);
			if (pathToIndexIt != m_pathToIndexMap.end())
			{
				return pathToIndexIt->second;
			}

			return {};
		}

		data::Id getId(std::filesystem::path const& a_path)
		{
			if (const auto existingId = findId(a_path))
			{
				return existingId.value();
			}

			return createId(a_path);
		}

		void insert(std::filesystem::path const& a_path, data::Id const a_id)
		{
			assert(!isRegistered(a_path));
			assert(!isRegistered(a_id));
			set(a_path, a_id);
			m_nextId = std::max(a_id + 1, m_nextId);
		}

	private:
		// Attributes
		data::Id m_nextId = 0;
		PathToIndexMap m_pathToIndexMap;
		IndexToPathMap m_indexToPathMap;

		// Methods
		data::Id createId(std::filesystem::path const& a_path)
		{
			auto const dataId = m_nextId;
			insert(a_path, dataId);
			return dataId;
		}

		void set(std::filesystem::path const& a_path, data::Id a_id)
		{
			auto const pathKey = PathKey{ a_path };
			m_pathToIndexMap.emplace(pathKey, a_id);
			m_indexToPathMap.emplace(a_id, a_path);
		}
	};
}
