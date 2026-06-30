#pragma once

#include "vob/aoe/rendering/resources/GpuBuffer.h"
#include "vob/aoe/rendering/resources/GpuMaterial.h"
#include "vob/aoe/rendering/resources/GpuMesh.h"
#include "vob/aoe/rendering/resources/GpuShader.h"
#include "vob/aoe/rendering/resources/Registry.h"


namespace vob::aoegl
{
	struct GpuResourceRegistriesContext
	{
		std::shared_ptr<Registry<GpuBuffer>> bufferRegistry;
		std::shared_ptr<Registry<GpuMaterial>> materialRegistry;
		std::shared_ptr<Registry<GpuMesh>> meshRegistry;
		std::shared_ptr<Registry<GpuShader>> shaderRegistry;
	};
}
