#include "vob/aoe/rendering/data/ShaderSourceLoader.h"

#include "vob/aoe/debug/Check.h"

#include <vob/aoe/data/FilesystemUtils.h>

#include <format>
#include <regex>


namespace vob::aoegl
{
	namespace
	{
		std::string readSource(
			aoedt::FilesystemIndexer const& a_indexer,
			aoedt::Database<std::pmr::string> const& a_stringDatabase,
			std::filesystem::path const& a_path)
		{
			auto const source = a_stringDatabase.find(a_indexer.getRuntimeId(a_path));
			if (!VOB_AOE_CHECK_LOG(source != nullptr, "Shader source not found: {}.", a_path.generic_string()))
			{
				return {};
			}

			return std::string{ *source };
		}

		void expandIncludes(
			aoedt::FilesystemIndexer const& a_indexer,
			aoedt::Database<std::pmr::string> const& a_stringDatabase,
			std::filesystem::path const& a_path,
			std::string& a_source)
		{
			auto const includeRegex = std::regex(R"(#include\s*["<](.*?)[">])");
			auto match = std::smatch{};
			auto searchStart = a_source.cbegin();

			while (std::regex_search(searchStart, a_source.cend(), match, includeRegex))
			{
				auto const includedPath = aoedt::FilesystemUtils::normalize(match[1].str(), a_path);
				auto includedSource = readSource(a_indexer, a_stringDatabase, includedPath);
				expandIncludes(a_indexer, a_stringDatabase, includedPath, includedSource);

				auto const matchPos = match.position(0) + (searchStart - a_source.cbegin());
				a_source.replace(matchPos, match.length(0), includedSource);

				searchStart = a_source.cbegin() + matchPos + includedSource.size();
			}
		}
	}

	ShaderSourceLoader::ShaderSourceLoader(
		aoedt::FilesystemIndexer const& a_indexer, aoedt::Database<std::pmr::string> const& a_stringDatabase)
		: m_indexer{ a_indexer }
		, m_stringDatabase{ a_stringDatabase }
	{}

	ShaderSource ShaderSourceLoader::load(std::filesystem::path const& a_path) const
	{
		auto const path = a_path.lexically_relative(m_indexer.getDataPath());
		auto source = readSource(m_indexer, m_stringDatabase, path);
		expandIncludes(m_indexer, m_stringDatabase, path, source);
		return ShaderSource{ std::move(source) };
	}
}
