#pragma once

#include <vob/aoe/physics/components/rigidbody.h>
#include <vob/aoe/physics/components/motion_state.h>
#include <vob/aoe/physics/math_util.h>
#include <vob/aoe/physics/world_components/physics_world_component.h>

#include <vob/aoe/api.h>
#include <vob/aoe/engine/world_data_provider.h>
#include <vob/aoe/spacetime/transform.h>


#include <vob/misc/std/ignorable_assert.h>


namespace vob::aoeph
{
	class rigidbody_system
	{
		static constexpr float k_physicsLimit = 1.0e6;
	public:
		explicit rigidbody_system(aoeng::world_data_provider& a_wdp)
			: m_physicsWorldComponent{ a_wdp }
			, m_dynamicRigidbodyEntities{ a_wdp }
		{
			a_wdp.on_construct<&rigidbody_system::on_spawn, rigidbody>(this);
			a_wdp.on_destroy<&rigidbody_system::on_despawn, rigidbody>(this);
		}

		void update() const
		{
			auto dynamicRigidbodyEntities = m_dynamicRigidbodyEntities.get();
			for (auto rigidbodyEntity : dynamicRigidbodyEntities)
			{
				auto [position, rotation, rigidbody, motionState] = dynamicRigidbodyEntities.get(rigidbodyEntity);

				btTransform btLocalTransform;
				motionState.m_instance->getWorldTransform(btLocalTransform);

#pragma message(VOB_MISTD_TODO "translate by -offset")
				auto const worldTransform = to_glm(btLocalTransform);
				
				position = worldTransform[3];
				rotation = glm::quat_cast(worldTransform);

				ignorable_assert(glm::length(glm::vec3{ position }) < k_physicsLimit);
			}
		}

		void on_spawn(aoeng::registry& a_registry, aoeng::entity a_entity) const
		{
			auto const position = a_registry.try_get<aoest::position>(a_entity);
			if (position == nullptr)
			{
				return;
			}

			auto const rotation = a_registry.try_get<aoest::rotation>(a_entity);
			if (rotation == nullptr)
			{
				return;
			}

			auto& rigidbodyCmp = a_registry.get<rigidbody>(a_entity);
			if (rigidbodyCmp.m_collisionShape == nullptr)
			{
				return;
			}

			btTransform const transform = to_bt(aoest::combine(*position, *rotation));

			btRigidBody::btRigidBodyConstructionInfo constructionInfo{
				rigidbodyCmp.m_mass, nullptr /* motion state */, rigidbodyCmp.m_collisionShape.get() };
			constructionInfo.m_startWorldTransform = transform;
			btVector3 intertia{ 0.0, 0.0, 0.0 };
			if (!rigidbodyCmp.m_isStatic)
			{
				auto& motionState = a_registry.emplace<motion_state>(a_entity);
				motionState.m_instance = std::make_unique<btDefaultMotionState>(
					transform, to_bt(rigidbodyCmp.m_centerOfMassOffset));
				constructionInfo.m_motionState = motionState.m_instance.get();

				if (rigidbodyCmp.m_mass > 0.0f)
				{
					rigidbodyCmp.m_collisionShape->calculateLocalInertia(rigidbodyCmp.m_mass, constructionInfo.m_localInertia);
				}
			}

			if (rigidbodyCmp.m_material != nullptr)
			{
				constructionInfo.m_restitution = rigidbodyCmp.m_material->m_restitution;
				constructionInfo.m_friction = rigidbodyCmp.m_material->m_friction;
				constructionInfo.m_rollingFriction = rigidbodyCmp.m_material->m_rollingFriction;
				constructionInfo.m_spinningFriction = rigidbodyCmp.m_material->m_spinningFriction;
			}
			// TODO: use material info

			rigidbodyCmp.m_instance = std::make_unique<btRigidBody>(constructionInfo);

			m_physicsWorldComponent->m_world.get().addRigidBody(rigidbodyCmp.m_instance.get(), 1 << 0, -1);
		}

		void on_despawn(aoeng::registry& a_registry, aoeng::entity a_entity) const
		{
			auto& rigidbodyCmp = a_registry.get<rigidbody>(a_entity);
			if (rigidbodyCmp.m_instance == nullptr)
			{
				return;
			}

			m_physicsWorldComponent->m_world.get().removeRigidBody(rigidbodyCmp.m_instance.get());
			
		}

	private:
		aoeng::world_component_ref<physics_world_component> m_physicsWorldComponent;

		aoeng::registry_view_ref<aoest::position, aoest::rotation, rigidbody, motion_state> m_dynamicRigidbodyEntities;
	};
}
