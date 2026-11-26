#pragma once

#include "vob/aoe/rendering/CoreShaderSources.h"
#include "vob/aoe/rendering/GraphicTypes.h"
#include "vob/aoe/rendering/data/ShaderSourceLoader.h"
#include "vob/aoe/rendering/resources/GpuShader.h"
#include "vob/aoe/rendering/ShaderDefinition.h"

#include <vob/aoe/data/FilesystemDatabase.h>
#include <vob/aoe/data/FilesystemVisitorContext.h>
#include "vob/aoe/data/Id.h"
#include <vob/aoe/data/JsonAccept.h>
#include <vob/aoe/data/JsonFileLoader.h>
#include <vob/aoe/data/MultiDatabase.h>
#include <vob/aoe/data/SingleFileLoader.h>
#include <vob/aoe/data/StringLoader.h>

#include <vob/misc/type/factory.h>
#include <vob/misc/type/registry.h>
#include <vob/misc/visitor/applicator.h>
#include <vob/misc/visitor/json_reader.h>

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
			std::filesystem::path partialSourcePath;
		};

		struct PostProcessProgram
		{
			std::string_view name;
			GraphicId program;
			std::filesystem::path sourcePath;
		};

		DebugProgramContext()
		{
			multiDatabase.registerDatabase(shaderSourceDatabase);
		}

		misty::pmr::registry typeRegistry;
		misty::pmr::factory factory{ typeRegistry };
		aoedt::FilesystemIndexer filesystemIndexer;
		aoedt::MultiDatabase multiDatabase;
		misvi::pmr::applicator<false, misvi::pmr::json_reader<aoedt::FilesystemVisitorContext>> jsonLoadApplicator;
		aoedt::FilesystemVisitorContextFactory contextFactory{ factory, filesystemIndexer, multiDatabase };
		aoedt::FilesystemDatabase<aoedt::SingleFileLoader<aoedt::StringLoader>> stringDatabase{ filesystemIndexer };
		aoedt::FilesystemDatabase<ShaderSourceLoader> shaderSourceDatabase{
			filesystemIndexer, filesystemIndexer, stringDatabase };
		aoedt::FilesystemDatabase<aoedt::JsonFileLoader<CoreShaderSources, aoedt::FilesystemVisitorContextFactory>>
			coreShaderSourcesDatabase{ filesystemIndexer, jsonLoadApplicator, contextFactory };
		std::filesystem::path coreShaderSourcesPath;
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
