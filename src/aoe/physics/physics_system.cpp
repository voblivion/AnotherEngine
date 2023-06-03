#include <vob/aoe/physics/physics_system.h>

#include <vob/aoe/physics/debug_drawer.h>

#include <vob/misc/std/ignorable_assert.h>

#include <glm/gtc/type_ptr.hpp>


namespace
{
	constexpr float k_physicLimit = 1.0e6;

	glm::mat4 to_glm(btTransform const& a_btTransform)
	{
		glm::mat4 glmTransform;
		a_btTransform.getOpenGLMatrix(glm::value_ptr(glmTransform));
		return glmTransform;
	}

	glm::mat4 get_transform(vob::aoeph::rigidbody_component const& a_rigidbody)
	{
		auto transform = btTransform();
		a_rigidbody.m_motionState->getWorldTransform(transform);
		return to_glm(transform);
	}

	void update_transform(
		vob::aoeph::rigidbody_component const& a_rigidbody,
		vob::aoest::transform_component& a_transform)
	{
		a_transform.m_matrix = get_transform(a_rigidbody);

		ignorable_assert(glm::length(glm::vec3(a_transform.m_matrix[3])) < k_physicLimit);
	}
}

namespace vob::aoeph
{
	physics_system::physics_system(aoeng::world_data_provider& a_wdp)
		: m_physicsWorldComponent{ a_wdp }
		, m_simulationTimeWorldComponent{ a_wdp }
		, m_rigidbodyEntities{ a_wdp }
	{}

	void physics_system::update() const
	{
		step_physic_world();
		update_transforms();
		try_debug_draw();
	}

	void physics_system::step_physic_world() const
	{
		auto& physicsWorld = m_physicsWorldComponent->m_world.get();
		physicsWorld.stepSimulation(m_simulationTimeWorldComponent->m_elapsedTime.get_value());
	}

	void physics_system::update_transforms() const
	{
		auto rigidbodyEntityView = m_rigidbodyEntities.get();
		for (auto rigidbodyEntity : rigidbodyEntityView)
		{
			auto [transform, rigidbody] = rigidbodyEntityView.get(rigidbodyEntity);
			update_transform(rigidbody, transform);
		}
	}

	void physics_system::try_debug_draw() const
	{
		if (m_physicsWorldComponent->m_enableDebugDraw)
		{
			debug_drawer debugDrawer;

			auto& physicsWorld = m_physicsWorldComponent->m_world.get();
			physicsWorld.setDebugDrawer(&debugDrawer);
			physicsWorld.debugDrawWorld();
			physicsWorld.setDebugDrawer(nullptr);
		}
	}
}
