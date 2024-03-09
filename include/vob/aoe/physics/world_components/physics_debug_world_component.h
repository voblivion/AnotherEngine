#pragma once

#include <vob/aoe/input/bindings.h>

#include <bullet/LinearMath/btIDebugDraw.h>


namespace vob::aoeph
{
	constexpr std::array<btIDebugDraw::DebugDrawModes, 5> k_debugDrawModes = {
		btIDebugDraw::DBG_NoDebug,
		btIDebugDraw::DebugDrawModes(btIDebugDraw::DBG_DrawAabb | btIDebugDraw::DBG_DrawConstraints),
		btIDebugDraw::DebugDrawModes(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawConstraints),
		btIDebugDraw::DebugDrawModes(btIDebugDraw::DBG_DrawAabb | btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawConstraints),
		btIDebugDraw::DebugDrawModes(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawNormals | btIDebugDraw::DBG_DrawConstraints)
	};

	struct physics_debug_world_component
	{
		uint32_t m_debugDrawModeIndex = 0;
		aoein::bindings::switch_id m_cycleDebugDrawModeBinding = 0;
	};
}
