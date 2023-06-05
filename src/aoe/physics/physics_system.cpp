#include <vob/aoe/physics/physics_system.h>

#include <vob/misc/std/ignorable_assert.h>

#include <bullet/BulletCollision/CollisionShapes/btBoxShape.h>
#include <bullet/BulletCollision/CollisionShapes/btPolyhedralConvexShape.h>

#include <glm/gtc/type_ptr.hpp>

#pragma message(VOB_MISTD_TODO "remove me, just debugging natvis issues.")
#include <glm/mat3x2.hpp>

namespace
{
	constexpr float k_physicLimit = 1.0e6;
	constexpr double k_g = 9.81;

	static const btBoxShape k_defaultShape = btBoxShape{ btVector3{0.5, 0.5, 0.5} };
	static const vob::aoeph::material k_defaultMaterial = {};

	btVector3 to_bt(glm::vec3 const& a_glmVector3)
	{
		return btVector3{ a_glmVector3.x, a_glmVector3.y, a_glmVector3.z };
	}

	btTransform to_bt(glm::mat4 const& a_glmTransform)
	{
		btTransform btTransformMatrix;
		btTransformMatrix.setFromOpenGLMatrix(glm::value_ptr(a_glmTransform));
		return btTransformMatrix;
	}

	glm::vec3 to_glm(btVector3 const& a_btVector)
	{
		return glm::vec3{ a_btVector[0], a_btVector[1], a_btVector[2] };
	}

	glm::mat4 to_glm(btTransform const& a_btTransform)
	{
		glm::mat4 glmTransform;
		a_btTransform.getOpenGLMatrix(glm::value_ptr(glmTransform));
		return glmTransform;
	}

	class debug_drawer final
		: public btIDebugDraw
	{
	public:
		explicit debug_drawer(vob::aoegl::debug_mesh_world_component& a_debugMeshWorldComponent)
			: m_debugMeshWorldComponent{ a_debugMeshWorldComponent }
		{}

		void drawLine(
			btVector3 const& a_source
			, btVector3 const& a_target
			, btVector3 const& a_color) override
		{
			m_debugMeshWorldComponent.add_line(
				vob::aoegl::debug_vertex{ to_glm(a_source), vob::aoegl::to_rgba(to_glm(a_color)) },
				vob::aoegl::debug_vertex{ to_glm(a_target), vob::aoegl::to_rgba(to_glm(a_color)) });
		}

		void drawContactPoint(
			btVector3 const& a_position
			, btVector3 const& a_normal
			, btScalar a_distance
			, int a_lifetime
			, btVector3 const& a_color) override
		{

		}

		void reportErrorWarning(char const* a_warningStr) override
		{

		}

		void draw3dText(btVector3 const& a_position, char const* a_textStr) override
		{

		}

		void setDebugMode(int a_debugMode) override
		{
			m_debugModes = static_cast<DebugDrawModes>(a_debugMode);
		}

		int getDebugMode() const override
		{
			return m_debugModes;
		}

	private:
		vob::aoegl::debug_mesh_world_component& m_debugMeshWorldComponent;
		DebugDrawModes m_debugModes = DBG_NoDebug;
	};
}

namespace vob::aoeph
{
	physics_system::physics_system(aoeng::world_data_provider& a_wdp)
		: m_physicsWorldComponent{ a_wdp }
		, m_simulationTimeWorldComponent{ a_wdp }
		, m_colliderEntities{ a_wdp }
		, m_debugMeshWorldComponent{ a_wdp }
		, m_bindings{ a_wdp }
	{
		a_wdp.on_construct<&physics_system::on_spawn, aoest::transform_component, collider_component>(this);
		a_wdp.on_destroy<&physics_system::on_despawn, rigidbody_component>(this);
	}

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
		auto colliderEntities = m_colliderEntities.get();
		for (auto rigidbodyEntity : colliderEntities)
		{
			auto [transform, collider, rigidbody] = colliderEntities.get(rigidbodyEntity);
			update_transform(collider, rigidbody, transform);
		}
	}

	void physics_system::update_transform(
		collider_component const& a_collider,
		motion_state_component const& a_motionState,
		aoest::transform_component& a_transform) const
	{
		auto btTransformMatrix = btTransform();
		a_motionState.m_instance->getWorldTransform(btTransformMatrix);
		
		a_transform.m_matrix = glm::translate(to_glm(btTransformMatrix), -a_collider.m_offset);

		ignorable_assert(glm::length(glm::vec3(a_transform.m_matrix[3])) < k_physicLimit);
	}

	void physics_system::try_debug_draw() const
	{
		auto const& cycleDebugDrawModeSwitch = *m_bindings->switches.find(
			m_physicsWorldComponent->m_cycleDebugDrawModeBinding);
		if (cycleDebugDrawModeSwitch.was_pressed())
		{
			m_physicsWorldComponent->m_debugDrawModeIndex =
				(m_physicsWorldComponent->m_debugDrawModeIndex + 1) % k_debugDrawModes.size();
		}
		if (m_physicsWorldComponent->m_debugDrawModeIndex != 0)
		{
			debug_drawer debugDrawer{ *m_debugMeshWorldComponent };
			debugDrawer.setDebugMode(k_debugDrawModes[m_physicsWorldComponent->m_debugDrawModeIndex]);
			
			auto& physicsWorld = m_physicsWorldComponent->m_world.get();
			physicsWorld.setDebugDrawer(&debugDrawer);
			physicsWorld.debugDrawWorld();
			physicsWorld.setDebugDrawer(nullptr);
		}
	}

	void physics_system::on_spawn(aoeng::entity_registry& a_registry, aoeng::entity a_entity) const
	{
		auto entity = aoeng::entity_handle{ a_registry, a_entity };
		auto& dynamicsWorld = m_physicsWorldComponent->m_world.get();
		
		// rigidbody
		auto [transform, collider] = a_registry.try_get<aoest::transform_component, collider_component>(a_entity);
		if (transform != nullptr && collider != nullptr)
		{
			ignorable_assert(collider->m_shape != nullptr && "collider doesn't have a shape.");
			ignorable_assert(collider->m_material != nullptr && "collider doesn't have a material.");

			auto& motionState = create_motion_state(entity, *transform, *collider);
			auto& rigidbody = create_rigidbody(entity, *collider, motionState);

			dynamicsWorld.addRigidBody(rigidbody.m_instance.get());
			return;
		}
	}

	void physics_system::on_despawn(aoeng::entity_registry& a_registry, aoeng::entity a_entity) const
	{
		auto& dynamicsWorld = m_physicsWorldComponent->m_world.get();
		auto rigidbody = a_registry.try_get<rigidbody_component>(a_entity);
		if (rigidbody != nullptr)
		{
			dynamicsWorld.removeRigidBody(rigidbody->m_instance.get());
		}
	}

	physics_system::motion_state_component& physics_system::create_motion_state(
		aoeng::entity_handle a_entity,
		aoest::transform_component const& a_transform,
		collider_component const& a_collider) const
	{
		auto const btTransformMatrix = to_bt(a_transform.m_matrix);
		auto const btOffsetMatrix = to_bt(glm::translate(glm::mat4{ 1.0f }, a_collider.m_offset));

#pragma message(VOB_MISTD_TODO "support custom allocator.")
		return a_entity.emplace<motion_state_component>(
			std::make_shared<btDefaultMotionState>(btTransformMatrix, btOffsetMatrix));
	}

	physics_system::rigidbody_component& physics_system::create_rigidbody(
		vob::aoeng::entity_handle a_entity,
		vob::aoeph::collider_component const& a_collider,
		motion_state_component& a_motionState) const
	{
		auto* shape = a_collider.m_shape != nullptr ? a_collider.m_shape.get() : &k_defaultShape;
		auto const& material = a_collider.m_material != nullptr ? *a_collider.m_material : k_defaultMaterial;

		auto const mass = a_collider.m_mass;
		btVector3 inertia{ 0.0, 0.0, 0.0 };
		if (mass != btScalar{ 0.0 })
		{
			shape->calculateLocalInertia(mass, inertia);
		}

#pragma message(VOB_MISTD_HACK "for some reason shape cannot be const, but nobody will modify it.")
		auto rigidbody = std::make_shared<btRigidBody>(
			mass, a_motionState.m_instance.get(), const_cast<btCollisionShape*>(shape), inertia);

		rigidbody->setGravity(btVector3{ btScalar{ 0.0 }, btScalar{ -k_g }, btScalar{ 0.0 } });
		rigidbody->setLinearFactor(to_bt(a_collider.m_linearFactor));
		rigidbody->setAngularFactor(to_bt(a_collider.m_angularFactor));
		rigidbody->setRestitution(material.m_restitution);
		rigidbody->setFriction(material.m_friction);
		rigidbody->setRollingFriction(material.m_rollingFriction);
		rigidbody->setSpinningFriction(material.m_spinningFriction);
		rigidbody->setContactStiffnessAndDamping(
			material.m_contactStiffness, material.m_contactDamping);

		return a_entity.emplace<rigidbody_component>(std::move(rigidbody));
	}
}
