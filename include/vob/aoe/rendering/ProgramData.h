#pragma once

#include <vob/aoe/api.h>

#include <vob/misc/visitor/macros.h>

#include <memory>
#include <string>


namespace vob::aoegl
{
	struct ProgramData
	{
		std::shared_ptr<std::pmr::string const> vertexShaderSource;
		std::shared_ptr<std::pmr::string const> fragmentShaderSource;
	};
}

namespace vob::misvi
{
	VOB_MISVI_ACCEPT(aoegl::ProgramData)
	{
		VOB_MISVI_NVP("Vertex Shader", vertexShaderSource);
		VOB_MISVI_NVP("Fragment Shader", fragmentShaderSource);
		return true;
	}
}
