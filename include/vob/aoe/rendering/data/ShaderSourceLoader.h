#pragma once

#include "vob/aoe/rendering/data/ShaderSource.h"

#include <vob/aoe/data/Database.h>
#include <vob/aoe/data/FilesystemIndexer.h>

#include <filesystem>
#include <string>


namespace vob::aoegl
{
	class ShaderSourceLoader
	{
	public:
		ShaderSourceLoader(
			aoedt::FilesystemIndexer const& a_indexer, aoedt::Database<std::pmr::string> const& a_stringDatabase);

		ShaderSource load(std::filesystem::path const& a_path) const;

	private:
		aoedt::FilesystemIndexer const& m_indexer;
		aoedt::Database<std::pmr::string> const& m_stringDatabase;
	};
}
