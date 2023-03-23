#pragma once

#include <filesystem>


namespace vob::aoedt::filesystem_util
{
	namespace fs = std::filesystem;

	inline auto normalize(
		std::filesystem::path const& a_relativePath
		, std::filesystem::path const& a_referencePath)
	{
		return fs::weakly_canonical(fs::absolute(a_referencePath).parent_path() / a_relativePath);
	}
}
