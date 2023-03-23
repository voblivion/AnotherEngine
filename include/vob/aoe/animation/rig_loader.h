#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/animation/rig_data.h>

#include <vob/aoe/data/database.h>
#include <vob/aoe/data/filesystem_indexer.h>

#include <filesystem>


namespace vob::aoegl
{
	struct VOB_AOE_API rig_loader
	{
		rig_data load(std::filesystem::path const& a_path) const;

		aoedt::filesystem_indexer const& m_indexer;
	};
}
