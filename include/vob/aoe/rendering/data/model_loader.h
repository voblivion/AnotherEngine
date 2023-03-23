#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/data/model_data.h>

#include <vob/aoe/data/database.h>
#include <vob/aoe/data/filesystem_indexer.h>

#include <filesystem>


namespace vob::aoegl
{
	struct VOB_AOE_API model_loader
	{
		model_data load(std::filesystem::path const& a_path) const;

		aoedt::database<texture_data> const& m_textureDatabase;
		aoedt::filesystem_indexer const& m_indexer;
	};
}
