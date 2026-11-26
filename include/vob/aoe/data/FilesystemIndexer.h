#pragma once

#include "vob/aoe/debug/Check.h"

#include <vob/aoe/data/Id.h>

#include <atomic>
#include <filesystem>
#include <format>
#include <mutex>
#include <unordered_map>


namespace vob::aoedt
{
	class FilesystemIndexer
	{
	public:
		using IdPathMap = std::pmr::unordered_map<Id, std::filesystem::path>;
		using PathIdMap = std::pmr::unordered_map<std::filesystem::path, Id>;

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
			VOB_AOE_CHECK_LOG(a_path.is_absolute(), "Path is not absolute: {}.", a_path.generic_string());
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
		mutable std::mutex m_mutex;
		mutable Id m_nextId = 0;
		mutable PathIdMap m_pathIdMap;
		mutable IdPathMap m_idPathMap;
	};
}
