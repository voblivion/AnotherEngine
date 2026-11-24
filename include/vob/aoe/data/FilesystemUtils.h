#pragma once

#include <filesystem>


namespace vob::aoedt::FilesystemUtils
{
	namespace fs = std::filesystem;

	inline auto normalize(
		std::filesystem::path const& a_relativePath
		, std::filesystem::path const& a_referencePath)
	{
		return (a_referencePath.parent_path() / a_relativePath).lexically_normal();
	}
}
