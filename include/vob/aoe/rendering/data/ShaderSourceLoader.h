#pragma once

#include "vob/aoe/rendering/data/ShaderSource.h"

#include <vob/aoe/data/Database.h>
#include <vob/aoe/data/FilesystemIndexer.h>

#include <filesystem>
#include <set>
#include <string>
#include <typeindex>
#include <utility>


namespace vob::aoegl
{
	class ShaderSourceLoader
	{
	public:
		ShaderSourceLoader(
			aoedt::FilesystemIndexer const& a_indexer, aoedt::Database<std::pmr::string> const& a_stringDatabase);

		ShaderSource load(std::filesystem::path const& a_path) const;

		void collectDependencies(
			std::filesystem::path const& a_path,
			std::set<std::pair<std::type_index, std::filesystem::path>>& o_dependencies) const;

	private:
		aoedt::FilesystemIndexer const& m_indexer;
		aoedt::Database<std::pmr::string> const& m_stringDatabase;
	};
}
