#include "vob/aoe/rendering/data/ShaderSourceLoader.h"

#include "vob/aoe/debug/Check.h"

#include <vob/aoe/data/FilesystemUtils.h>

#include <algorithm>
#include <cctype>
#include <format>
#include <memory>
#include <optional>
#include <ranges>
#include <string_view>
#include <vector>


namespace vob::aoegl
{
	namespace
	{
		constexpr std::string_view k_includeDirective = "#include";

		struct IncludeDirective
		{
			std::size_t begin;
			std::size_t end;
			std::string_view path;
		};

		std::optional<IncludeDirective> parseIncludePath(std::string_view a_line, std::size_t a_begin)
		{
			auto const open = a_line.find_first_not_of(" \t", a_begin + k_includeDirective.size());
			if (open == std::string_view::npos || (a_line[open] != '"' && a_line[open] != '<'))
			{
				return std::nullopt;
			}

			auto const close = a_line.find(a_line[open] == '"' ? '"' : '>', open + 1);
			if (close == std::string_view::npos)
			{
				return std::nullopt;
			}

			return IncludeDirective{ a_begin, close + 1, a_line.substr(open + 1, close - open - 1) };
		}

		std::optional<IncludeDirective> parseInclude(std::string_view a_line, bool& a_isInBlockComment)
		{
			auto include = std::optional<IncludeDirective>{};
			auto isDirectiveAllowed = true;
			auto i = std::size_t{ 0 };
			while (i < a_line.size())
			{
				auto const rest = a_line.substr(i);
				if (a_isInBlockComment)
				{
					if (rest.starts_with("*/"))
					{
						a_isInBlockComment = false;
						i += 2;
					}
					else
					{
						++i;
					}
				}
				else if (rest.starts_with("//"))
				{
					break;
				}
				else if (rest.starts_with("/*"))
				{
					a_isInBlockComment = true;
					i += 2;
				}
				else if (std::isspace(static_cast<unsigned char>(rest.front())))
				{
					++i;
				}
				else
				{
					if (isDirectiveAllowed && rest.starts_with(k_includeDirective))
					{
						include = parseIncludePath(a_line, i);
					}
					isDirectiveAllowed = false;
					i = include.has_value() ? include->end : i + 1;
				}
			}

			return include;
		}

		std::shared_ptr<std::pmr::string const> readSource(
			aoedt::FilesystemIndexer const& a_indexer,
			aoedt::Database<std::pmr::string> const& a_stringDatabase,
			std::filesystem::path const& a_path)
		{
			auto source = a_stringDatabase.find(a_indexer.getRuntimeId(a_path));
			VOB_AOE_CHECK_LOG(source != nullptr, "Shader source not found: {}.", a_path.generic_string());
			return source;
		}

		std::string expandIncludes(
			aoedt::FilesystemIndexer const& a_indexer,
			aoedt::Database<std::pmr::string> const& a_stringDatabase,
			std::filesystem::path const& a_path,
			std::vector<std::filesystem::path>& a_includeStack)
		{
			if (!VOB_AOE_CHECK_LOG(
					std::ranges::find(a_includeStack, a_path) == a_includeStack.end(),
					"Shader include cycle: {}.", a_path.generic_string()))
			{
				return {};
			}
			a_includeStack.push_back(a_path);

			auto const source = readSource(a_indexer, a_stringDatabase, a_path);
			if (source == nullptr)
			{
				a_includeStack.pop_back();
				return {};
			}

			auto expandedSource = std::string{};
			auto isInBlockComment = false;
			for (auto const lineRange : std::string_view{ *source } | std::views::split('\n'))
			{
				auto const line = std::string_view{ lineRange.begin(), lineRange.end() };
				auto const include = parseInclude(line, isInBlockComment);
				if (include.has_value())
				{
					expandedSource += line.substr(0, include->begin);
					expandedSource += expandIncludes(
						a_indexer,
						a_stringDatabase,
						aoedt::FilesystemUtils::normalize(include->path, a_path),
						a_includeStack);
					expandedSource += line.substr(include->end);
				}
				else
				{
					expandedSource += line;
				}
				expandedSource += '\n';
			}

			a_includeStack.pop_back();
			return expandedSource;
		}
	}

	ShaderSourceLoader::ShaderSourceLoader(
		aoedt::FilesystemIndexer const& a_indexer, aoedt::Database<std::pmr::string> const& a_stringDatabase)
		: m_indexer{ a_indexer }
		, m_stringDatabase{ a_stringDatabase }
	{}

	ShaderSource ShaderSourceLoader::load(std::filesystem::path const& a_path) const
	{
		auto includeStack = std::vector<std::filesystem::path>{};
		return ShaderSource{ expandIncludes(m_indexer, m_stringDatabase, a_path, includeStack) };
	}

	void ShaderSourceLoader::collectDependencies(
		std::filesystem::path const& a_path,
		std::set<std::pair<std::type_index, std::filesystem::path>>& o_dependencies) const
	{
		auto const source = readSource(m_indexer, m_stringDatabase, a_path);
		if (source == nullptr)
		{
			return;
		}

		auto isInBlockComment = false;
		for (auto const lineRange : std::string_view{ *source } | std::views::split('\n'))
		{
			auto const include = parseInclude(std::string_view{ lineRange.begin(), lineRange.end() }, isInBlockComment);
			if (!include.has_value())
			{
				continue;
			}

			auto const includedPath = aoedt::FilesystemUtils::normalize(include->path, a_path);
			if (o_dependencies.emplace(typeid(ShaderSource), includedPath).second)
			{
				collectDependencies(includedPath, o_dependencies);
			}
		}
	}
}
