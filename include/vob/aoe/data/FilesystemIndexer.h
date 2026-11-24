#pragma once

#include <vob/aoe/data/Id.h>

#include <atomic>
#include <cassert>
#include <filesystem>
#include <mutex>
#include <unordered_map>


namespace vob::aoedt
{
	class FilesystemIndexer
	{
	public:
		using IdPathMap = std::pmr::unordered_map<Id, std::filesystem::path>;
		using PathIdMap = std::pmr::unordered_map<std::filesystem::path, Id>;

		explicit FilesystemIndexer(std::filesystem::path a_dataPath)
			: m_dataPath{ std::move(a_dataPath) }
		{}

		auto const& getDataPath() const
		{
			return m_dataPath;
		}

		std::filesystem::path const* findPath(Id const a_id) const
		{
			std::lock_guard<std::mutex> lock{ m_mutex };
			auto const pathIt = m_idPathMap.find(a_id);
			if (pathIt != m_idPathMap.end())
			{
				return &(pathIt->second);
			}
			return nullptr;
		}

		Id getRuntimeId(std::filesystem::path const& a_path) const
		{
			auto const path = a_path.lexically_normal();
			std::lock_guard<std::mutex> lock{ m_mutex };
			auto const idIt = m_pathIdMap.find(path);
			if (idIt != m_pathIdMap.end())
			{
				return idIt->second;
			}

			auto const runtimeId = m_nextId++;
			m_pathIdMap.emplace(path, runtimeId);
			m_idPathMap.emplace(runtimeId, path);
			return runtimeId;
		}

	private:
		std::filesystem::path m_dataPath;
		mutable std::mutex m_mutex;
		mutable Id m_nextId = 0;
		mutable PathIdMap m_pathIdMap;
		mutable IdPathMap m_idPathMap;
	};
}
