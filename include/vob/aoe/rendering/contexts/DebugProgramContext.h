#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"
#include "vob/aoe/rendering/resources/GpuShader.h"
#include "vob/aoe/rendering/ShaderDefinition.h"

#include <vob/aoe/data/FilesystemDatabase.h>
#include "vob/aoe/data/Id.h"
#include <vob/aoe/data/SingleFileLoader.h>
#include <vob/aoe/data/StringLoader.h>

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
			std::weak_ptr<GpuShader> shader;
			std::shared_ptr<ShaderDefinition const> shaderDefinition;
		};

		struct PostProcessProgram
		{
			std::string_view name;
			GraphicId program;
			std::filesystem::path sourcePath;
		};

		explicit DebugProgramContext(std::filesystem::path a_dataPath)
			: filesystemIndexer{ std::move(a_dataPath) }
		{}

		aoedt::FilesystemIndexer filesystemIndexer;
		aoedt::FilesystemDatabase<aoedt::SingleFileLoader<aoedt::StringLoader>> stringDatabase{ filesystemIndexer };
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
