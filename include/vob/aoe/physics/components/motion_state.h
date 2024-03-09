#pragma once

#include <vob/aoe/physics/math_util.h>

#include <vob/aoe/engine/world_data.h>

#include <bullet/LinearMath/btDefaultMotionState.h>

namespace vob::aoeph
{
	struct motion_state
	{
		std::unique_ptr<btDefaultMotionState> m_instance = nullptr;
	};
}
