#pragma once

#include "vob/aoe/debug/Check.h"

#include <filesystem>
#include <format>
#include <fstream>
#include <utility>


namespace vob::aoedt
{
	template <typename TStreamLoader>
	class SingleFileLoader
	{
	public:
		explicit SingleFileLoader(TStreamLoader&& a_loader)
			: m_streamLoader{ a_loader }
		{}

		template <typename... TArgs>
		explicit SingleFileLoader(TArgs&&... a_args)
			: m_streamLoader{ std::forward<TArgs>(a_args)... }
		{}

		auto load(std::filesystem::path const& a_path) const
		{
			auto file = std::ifstream{ a_path, std::ios::binary | std::ios::in };
			VOB_AOE_CHECK_LOG(file.is_open(), "Failed to open file: {}.", a_path.generic_string());
			return m_streamLoader.load(file);
		}

		TStreamLoader m_streamLoader;
	};
}
