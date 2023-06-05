#pragma once

#include <vob/aoe/input/bindings.h>

#include <bullet/BulletDynamics/Dynamics/btDynamicsWorld.h>
#include <bullet/LinearMath/btIDebugDraw.h>

#include <array>


namespace vob::aoeph
{
	constexpr std::array<btIDebugDraw::DebugDrawModes, 5> k_debugDrawModes = {
		btIDebugDraw::DBG_NoDebug,
		btIDebugDraw::DBG_DrawAabb,
		btIDebugDraw::DBG_DrawWireframe,
		btIDebugDraw::DebugDrawModes(btIDebugDraw::DBG_DrawAabb | btIDebugDraw::DBG_DrawWireframe),
		btIDebugDraw::DebugDrawModes(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawNormals)
	};

	class physics_world_component
	{
	public:
		explicit physics_world_component(btDynamicsWorld& a_physicWorld)
			: m_world{ a_physicWorld }
		{}

		std::reference_wrapper<btDynamicsWorld> m_world;
		bool m_isPaused = false;
		uint32_t m_debugDrawModeIndex = 0;
		aoein::bindings::switch_id m_cycleDebugDrawModeBinding = 0;
	};
}
