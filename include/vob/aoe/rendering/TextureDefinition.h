#pragma once

#include "vob/aoe/rendering/TextureSettings.h"

#include <filesystem>


namespace vob::aoegl
{
	struct TextureDefinition
	{
		std::filesystem::path path;
		TextureSettings settings;
	};
}
