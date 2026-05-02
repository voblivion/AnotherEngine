#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"

#include <vob/aoe/data/filesystem_database.h>
#include "vob/aoe/data/id.h"
#include <vob/aoe/data/single_file_loader.h>
#include <vob/aoe/data/string_loader.h>

#include <filesystem>
#include <memory>
#include <string_view>
#include <vector>


namespace vob::aoegl
{
	struct DebugProgramContext
	{
		struct ForwardProgram
		{
			std::string_view name;
			GraphicId staticProgram;
			GraphicId riggedProgram;
			GraphicId instancedProgram;
			std::filesystem::path shadingSourcePath;
		};

		struct PostProcessProgram
		{
			std::string_view name;
			GraphicId program;
			std::filesystem::path sourcePath;
		};

		aoedt::filesystem_indexer filesystemIndexer;
		aoedt::filesystem_database<aoedt::single_file_loader<aoedt::string_loader>> stringDatabase{ filesystemIndexer };
		std::vector<ForwardProgram> forwardPrograms;
		GraphicId ssaoProgram;
		GraphicId ssrProgram;
		GraphicId opaqueCompositionProgram;
		GraphicId skyBoxProgram;
		std::filesystem::path skyBoxSourcePath;
	};
}
