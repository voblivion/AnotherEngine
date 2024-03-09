#pragma once

#include <bullet/BulletDynamics/Dynamics/btDynamicsWorld.h>

#include <array>


namespace vob::aoeph
{
	class physics_world_component
	{
	public:
		explicit physics_world_component(btDynamicsWorld& a_physicWorld)
			: m_world{ a_physicWorld }
		{}

		std::reference_wrapper<btDynamicsWorld> m_world;
	};
}
