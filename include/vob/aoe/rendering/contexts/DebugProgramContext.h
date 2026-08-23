#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"
#include "vob/aoe/rendering/resources/GpuShader.h"
#include "vob/aoe/rendering/resources/WeakHandle.h"
#include "vob/aoe/rendering/ShaderDefinition.h"

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
		struct Shader
		{
			WeakHandle<GpuShader> shader;
			std::shared_ptr<ShaderDefinition const> shaderDefinition;
		};

		struct PostProcessProgram
		{
			std::string_view name;
			GraphicId program;
			std::filesystem::path sourcePath;
		};

		aoedt::filesystem_indexer filesystemIndexer;
		aoedt::filesystem_database<aoedt::single_file_loader<aoedt::string_loader>> stringDatabase{ filesystemIndexer };
		std::vector<Shader> shaders;
		int32_t activeShaderIndex = 0;
		GraphicId ssaoProgram;
		GraphicId ssrProgram;
		GraphicId opaqueCompositionProgram;
		GraphicId skyBoxProgram;
		GraphicId skyIrradianceProgram;
		std::filesystem::path skyPartialSourcePath;
	};
}
