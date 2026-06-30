#pragma once

#include "vob/aoe/editor/AGizmoHandle.h"

#include <memory>
#include <vector>


namespace vob::aoedi
{
	struct GizmoComponent
	{
		std::vector<std::shared_ptr<AGizmoHandle>> handles;
	};
}
