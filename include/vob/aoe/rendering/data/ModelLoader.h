#pragma once

#include <vob/aoe/api.h>

#include "vob/aoe/rendering/data/ModelData.h"

#include <filesystem>


namespace vob::aoegl
{
	struct StaticModelLoader
	{
		StaticModelData load(std::filesystem::path const& a_path) const;
	};

	struct RiggedModelLoader
	{
		RiggedModelData load(std::filesystem::path const& a_path) const;
	};
}
