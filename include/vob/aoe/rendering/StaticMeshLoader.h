#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/StaticMesh.h>

#include <filesystem>


namespace vob::aoegl
{
	struct VOB_AOE_API StaticMeshLoader
	{
		StaticMesh load(std::filesystem::path const& a_path) const;
	};
}
