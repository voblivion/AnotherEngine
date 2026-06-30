#pragma once

#include "vob/aoe/editor/AGizmoHandle.h"

#include <memory>


namespace vob::aoedi
{
	struct GizmoContext
	{
		std::weak_ptr<AGizmoHandle> hoveredHandle;
		std::weak_ptr<AGizmoHandle> activeHandle;
	};
}
