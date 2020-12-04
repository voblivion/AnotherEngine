#pragma once

#include <filesystem>
#include <functional>

namespace vob::aoe::common
{
	namespace fs = std::filesystem;

	inline auto pathFromFilePath(
		std::filesystem::path const& a_relativePath
		, std::filesystem::path const& a_filePath
	)
	{
		return fs::absolute(a_filePath).parent_path() / a_relativePath;
	}

	inline auto cleanPath(
		std::filesystem::path const& a_path
		, std::filesystem::path const& a_base = std::filesystem::current_path()
	)
	{
		auto cleanPath = std::filesystem::relative(std::filesystem::weakly_canonical(a_path), a_base);
		cleanPath.make_preferred();
		return cleanPath;
	}

	class PathKey
	{
	public:
		// Constructors
		explicit PathKey(std::filesystem::path const& a_path)
			: m_cleanPath{ cleanPath(a_path) }
			, m_hashValue{ std::hash_value(m_cleanPath.native()) }
		{}

		auto getHashValue() const
		{
			return m_hashValue;
		}

		// Operators
		friend bool operator==(PathKey const& a_lhs, PathKey const& a_rhs)
		{
			return a_lhs.m_cleanPath == a_rhs.m_cleanPath;
		}

	private:
		// Attributes
		std::filesystem::path m_cleanPath;
		std::size_t m_hashValue;
	};
}

namespace std
{
	template <>
	struct hash<vob::aoe::common::PathKey>
	{
		std::size_t operator()(const vob::aoe::common::PathKey& a_pathKey) const noexcept
		{
			return a_pathKey.getHashValue();
		}
	};
}
