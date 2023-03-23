#pragma once

#include <vob/aoe/data/id.h>

#include <vob/misc/std/filesystem.h>

#include <atomic>
#include <cassert>
#include <filesystem>
#include <mutex>
#include <unordered_map>


namespace vob::aoedt
{
	class filesystem_indexer
	{
	public:
		using id_path_map = std::pmr::unordered_map<id, std::filesystem::path>;
		using path_id_map = std::pmr::unordered_map<std::filesystem::path, id>;

		std::filesystem::path const* find_path(id const a_id) const
		{
			std::lock_guard<std::mutex> lock{ m_mutex };
			auto const pathIt = m_idPathMap.find(a_id);
			if (pathIt != m_idPathMap.end())
			{
				return &(pathIt->second);
			}
			return nullptr;
		}

		id get_runtime_id(std::filesystem::path const& a_path) const
		{
			std::lock_guard<std::mutex> lock{ m_mutex };
			auto const idIt = m_pathIdMap.find(a_path);
			if (idIt != m_pathIdMap.end())
			{
				return idIt->second;
			}
			
			auto const runtimeId = m_nextId++;
			m_pathIdMap.emplace(a_path, runtimeId);
			m_idPathMap.emplace(runtimeId, a_path);
			return runtimeId;
		}

	private:
		mutable std::mutex m_mutex;
		mutable id m_nextId = 0;
		mutable path_id_map m_pathIdMap;
		mutable id_path_map m_idPathMap;
	};
}
