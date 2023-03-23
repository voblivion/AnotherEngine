#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/data/texture_data.h>

#include <filesystem>


namespace vob::aoegl
{
	struct VOB_AOE_API texture_file_loader
	{
		texture_data load(std::filesystem::path const& a_path) const;
	};
}
