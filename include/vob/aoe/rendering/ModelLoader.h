#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/ModelData.h>

#include <filesystem>


namespace vob::aoegl
{
	struct VOB_AOE_API StaticModelLoader
	{
		StaticModelData load(std::filesystem::path const& a_path) const;
	};

	struct VOB_AOE_API RiggedModelLoader
	{
		RiggedModelData load(std::filesystem::path const& a_path) const;
	};
}
