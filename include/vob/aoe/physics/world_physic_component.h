#pragma once

#include <vob/aoe/physics/debug_drawer.h>

#include <bullet/BulletDynamics/Dynamics/btDynamicsWorld.h>

#include <bullet/LinearMath/btIDebugDraw.h>


namespace vob::aoeph
{
	class world_physic_component
	{
	public:
		explicit world_physic_component(btDynamicsWorld& a_physicWorld)
			: m_world{ a_physicWorld }
		{}

		std::reference_wrapper<btDynamicsWorld> m_world;
		bool m_isPaused = false;
		debug_drawer m_debugDrawer = {};
	};
}
