#pragma once

#include "vob/aoe/rendering/MaterialManager.h"

#include <memory>


namespace vob::aoegl
{
	struct MaterialManagerContext
	{
		std::shared_ptr<MaterialManager> materialManager;
	};
}
