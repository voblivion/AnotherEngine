#pragma once

#include <bullet/BulletDynamics/Dynamics/btDynamicsWorld.h>

#include <vob/misc/physics/measure.h>
#include <vob/misc/physics/measure_literals.h>

#include <array>
#include <chrono>


namespace vob::aoeph
{
	using namespace vob::misph::literals;

	class physics_world_component
	{
	public:
		explicit physics_world_component(btDynamicsWorld& a_physicWorld)
			: m_world{ a_physicWorld }
		{}

		std::reference_wrapper<btDynamicsWorld> m_world;
	};

	struct physx_context
	{
		using clock = std::chrono::high_resolution_clock;

		std::chrono::time_point<clock> m_lastUpdateTime;

		misph::measure_time m_updateDuration = 0.01_s;

		int32_t m_updateStepCount = 10;
	};

	struct physx_debug_context
	{
		bool is_dynamic_shape_debug_enabled = true;
		bool is_dynamic_velocity_debug_enabled = false;
		bool is_dynamic_contact_debug_enabled = false;
		bool is_static_shape_debug_enabled = false;
	};
}
