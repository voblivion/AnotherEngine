#pragma once

#include <vob/aoe/core/ecs/WorldDataProvider.h>
#include <vob/aoe/common/physic/CharacterControllerComponent.h>
#include <vob/aoe/common/physic/RigidBodyComponent.h>
#include <vob/aoe/common/physic/WorldPhysicComponent.h>
#include <vob/aoe/common/render/debugscene/DebugMesh.h>
#include <vob/aoe/common/render/debugscene/DebugSceneRenderComponent.h>
#include <vob/aoe/common/space/TransformComponent.h>
#include <vob/aoe/common/time/WorldTimeComponent.h>
#include <LinearMath/btIDebugDraw.h>

namespace vob::aoe::common
{
	struct DebugDrawer final
		: public btIDebugDraw
	{
		explicit DebugDrawer(DebugMesh& a_debugMesh)
			: m_debugMesh{ a_debugMesh }
		{}

		void drawLine(btVector3 const& a_source, btVector3 const& a_target
			, btVector3 const& a_color) override
		{
			m_debugMesh.addLine(
				DebugVertex{ toGlmVec3(a_source)
					, toGlmVec3(a_color) }
				, DebugVertex{ toGlmVec3(a_target)
					, toGlmVec3(a_color) });
		}

		void drawLine(btVector3 const& a_source, btVector3 const& a_target
			, btVector3 const& a_sourceColor
			, const btVector3& a_targetColor) override
		{
			m_debugMesh.addLine(
				DebugVertex{ toGlmVec3(a_source)
					, toGlmVec3(a_sourceColor) }
				, DebugVertex{ toGlmVec3(a_target)
					, toGlmVec3(a_targetColor) });
		}

		void drawContactPoint(const btVector3& PointOnB
			, const btVector3& normalOnB, btScalar distance, int lifeTime
			, const btVector3& color) override
		{
			
		}

		void reportErrorWarning(const char* warningString) override
		{
			
		}


		void draw3dText(const btVector3& location
			, const char* textString) override
		{
			
		}

		void setDebugMode(int a_debugMode) override
		{
			t_debugMode = a_debugMode;
		}

		int getDebugMode() const override
		{
			return DBG_DrawWireframe |  DBG_FastWireframe | DBG_DrawAabb;
		}

		std::uint32_t t_debugMode{ 1 };
		DebugMesh& m_debugMesh;
	};

	constexpr float g_physicLimit = 1.0e6;

	struct PhysicSystem
	{
		using RigidBodyComponents = ecs::ComponentTypeList<
			TransformComponent
			, RigidBodyComponent
		>;
		using CharacterComponents = ecs::ComponentTypeList<
			TransformComponent
			, CharacterControllerComponent
		>;
			
		explicit PhysicSystem(ecs::WorldDataProvider& a_wdp)
			: m_worldPhysicComponent{
				*a_wdp.getWorldComponent<WorldPhysicComponent>() }
			, m_worldTimeComponent{
				*a_wdp.getWorldComponent<WorldTimeComponent>() }
			, m_debugSceneRenderComponent{
				*a_wdp.getWorldComponent<DebugSceneRenderComponent>() }
			, m_rigidBodyEntities{
				a_wdp.getEntityViewList(*this, RigidBodyComponents{}) }
			, m_characterEntities{
				a_wdp.getEntityViewList(*this, CharacterComponents{}) }
		{}

		void update() const
		{
			auto& t_dynamicsWorld =
				m_worldPhysicComponent.m_dynamicsWorldHolder->getDynamicsWorld();

			if (!m_worldPhysicComponent.m_pause)
			{
				t_dynamicsWorld.stepSimulation(m_worldTimeComponent.m_elapsedTime.value);

				for (auto& t_rigidBodyEntity : m_rigidBodyEntities)
				{
					auto& t_transform = t_rigidBodyEntity.getComponent<TransformComponent>();
					auto& t_rigidBody = t_rigidBodyEntity.getComponent<RigidBodyComponent>();

					auto t_matrix = btTransform{};
					t_rigidBody.m_motionState->getWorldTransform(t_matrix);

					t_transform.m_matrix = toGlmMat4(t_rigidBody.m_motionState->m_graphicsWorldTrans);
					
					constexpr auto g_physicLimitSquared = g_physicLimit * g_physicLimit;
					ignorable_assert(squaredLength(getTranslation(t_transform.m_matrix)) < g_physicLimitSquared);
				}
			}

			if (m_worldPhysicComponent.m_displayDebug)
			{
				DebugDrawer t_debugDrawer{ m_debugSceneRenderComponent.m_debugMesh };
				t_dynamicsWorld.setDebugDrawer(&t_debugDrawer);
				t_dynamicsWorld.debugDrawWorld();
				t_dynamicsWorld.setDebugDrawer(nullptr);
			}
		}

		void onEntityAdded(ecs::Entity const& a_entity) const
		{
			ignorable_assert(m_worldPhysicComponent.m_dynamicsWorldHolder != nullptr);
			if (m_worldPhysicComponent.m_dynamicsWorldHolder == nullptr)
			{
				return;
			}
			auto& t_dynamicsWorld =
				m_worldPhysicComponent.m_dynamicsWorldHolder->getDynamicsWorld();

			// RigidBody
			if (auto const t_rigidBody
				= a_entity.getComponent<RigidBodyComponent>())
			{
				if(t_rigidBody->m_collisionShape == nullptr)
				{
					ignorable_assert(false);
					return;
				}

				// Initialize motion state
				auto const t_transform = a_entity.getComponent<TransformComponent>();
				auto const t_matrix = toBtTransform(t_transform->m_matrix);
				auto const t_offset = toBtTransform(glm::translate(mat4{ 1.0f }, t_rigidBody->m_offset));
				t_rigidBody->m_motionState = std::make_shared<btDefaultMotionState>(t_matrix, t_offset);

				// Compute inertia
				auto const t_mass = t_rigidBody != nullptr
					? t_rigidBody->m_mass
					: btScalar{ 0.0 };
				btVector3 t_inertia{ 0.0, 0.0, 0.0 };
				auto& t_shape = t_rigidBody->m_collisionShape->getCollisionShape();
				if (t_mass != btScalar{ 0.0 })
				{
					t_shape.calculateLocalInertia(t_mass, t_inertia);
				}

				// Create Bullet's rigid body
				// TODO allocator...
				t_rigidBody->m_rigidBody = std::make_shared<btRigidBody>(
					t_mass
					, t_rigidBody->m_motionState.get()
					, &t_shape
					, t_inertia
				);
				t_rigidBody->m_rigidBody->setGravity(btVector3{ 0.0, -9.81f, 0.0 });

				// Apply initial velocity
				t_rigidBody->m_rigidBody->setLinearVelocity(
					toBtVector(t_rigidBody->m_linearVelocity));

				t_rigidBody->m_rigidBody->setLinearFactor(
					toBtVector(t_rigidBody->m_linearFactor));
				t_rigidBody->m_rigidBody->setAngularFactor(
					toBtVector(t_rigidBody->m_angularFactor));


				// Apply material
				if (t_rigidBody->m_physicMaterial != nullptr)
				{
					t_rigidBody->m_rigidBody->setRestitution(
						t_rigidBody->m_physicMaterial->m_restitution);
					t_rigidBody->m_rigidBody->setFriction(
						t_rigidBody->m_physicMaterial->m_friction);
					t_rigidBody->m_rigidBody->setRollingFriction(
						t_rigidBody->m_physicMaterial->m_rollingFriction);
					t_rigidBody->m_rigidBody->setSpinningFriction(
						t_rigidBody->m_physicMaterial->m_spinningFriction);
					t_rigidBody->m_rigidBody->setContactStiffnessAndDamping(
						t_rigidBody->m_physicMaterial->m_contactStiffness
						, t_rigidBody->m_physicMaterial->m_contactDamping);
				}

				// Add rigid body to physic simulation
				t_dynamicsWorld.addRigidBody(&*t_rigidBody->m_rigidBody);
			}
			else if(auto const t_characterController
				= a_entity.getComponent<CharacterControllerComponent>())
			{
				t_characterController->m_kinematic = btKinematicCharacterController{
					&t_characterController->m_ghost
					, &t_characterController->m_capsule
					, 0.5f
				};

				t_dynamicsWorld.addAction(&*t_characterController->m_kinematic);
			}
		}

		void onEntityRemoved(ecs::Entity const& a_entity) const
		{
			ignorable_assert(m_worldPhysicComponent.m_dynamicsWorldHolder != nullptr);
			if (m_worldPhysicComponent.m_dynamicsWorldHolder == nullptr)
			{
				return;
			}
			auto& t_dynamicsWorld =
				m_worldPhysicComponent.m_dynamicsWorldHolder->getDynamicsWorld();

			auto const t_rigidBody = a_entity.getComponent<RigidBodyComponent>();
			if (t_rigidBody != nullptr)
			{
				t_dynamicsWorld.removeRigidBody(t_rigidBody->m_rigidBody.get());
			}
		}

	private:
		
		WorldPhysicComponent& m_worldPhysicComponent;
		WorldTimeComponent& m_worldTimeComponent;
		DebugSceneRenderComponent& m_debugSceneRenderComponent;

		ecs::EntityViewList<
			TransformComponent
			, RigidBodyComponent
		> const& m_rigidBodyEntities;
		ecs::EntityViewList<
			TransformComponent
			, CharacterControllerComponent
		> const& m_characterEntities;
	};
}
