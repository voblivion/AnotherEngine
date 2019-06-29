#pragma once
#include <aoe/core/ecs/WorldDataProvider.h>
#include <aoe/common/map/HierarchyComponent.h>
#include <aoe/common/space/TransformComponent.h>
#include <aoe/common/space/HierarchyTransformComponent.h>

namespace aoe
{
	namespace common
	{
		class HierarchyTransformSystem final
		{
		public:
			using Components = ecs::ComponentTypeList<HierarchyComponent const*
				, TransformComponent, HierarchyTransformComponent>;

			explicit HierarchyTransformSystem(ecs::WorldDataProvider& a_worldDataProvider)
				: m_entities{ a_worldDataProvider.getEntityList(Components{}) }
			{}

			void update() const
			{
				for (auto const& t_entity : m_entities)
				{
					auto& t_hierarchyTransform = t_entity.getComponent<
						HierarchyTransformComponent>();
					if(!t_hierarchyTransform.m_processed)
					{
						process(t_entity);
					}

					/*if(isNullWithEpsilon(t_linearMove)
						|| isNullWithEpsilon(t_angularMove))
					{
						
					}*/
				}

				for (auto const& t_entity : m_entities)
				{
					auto& t_hierarchyTransform = t_entity.getComponent<
						HierarchyTransformComponent>();
					auto& t_transform = t_entity.getComponent<
						TransformComponent>();

					t_hierarchyTransform.m_processed = false;
					t_hierarchyTransform.m_oldPosition = t_transform.m_position;
					t_hierarchyTransform.m_oldRotation = t_transform.m_rotation;
				}
			}

		private:
			// Attributes
			ecs::SystemEntityList<HierarchyComponent const*, TransformComponent
				, HierarchyTransformComponent> const& m_entities;

			using Entity = ecs::SystemEntity<HierarchyComponent const*
				, TransformComponent, HierarchyTransformComponent>;

			// Methods
			void process(Entity const& a_entity, Vector3 a_linearMove = {}
				, Quaternion const a_angularMove = { 1.0f, 0.0f, 0.0f, 0.0f }
				, Vector3 const a_referencePosition = {}) const
			{
				auto t_hierarchy = a_entity.getComponent<HierarchyComponent>();
				auto& t_transform = a_entity.getComponent<TransformComponent>();
				auto& t_hierarchyTransform = a_entity.getComponent<
					HierarchyTransformComponent>();

				assert(!t_hierarchyTransform.m_processed);
				t_hierarchyTransform.m_processed = true;

				auto t_linearMove = t_transform.m_position
					- t_hierarchyTransform.m_oldPosition;
				auto t_angularMove = t_transform.m_rotation
					* glm::inverse(t_hierarchyTransform.m_oldRotation);

				auto t_offset = t_hierarchyTransform.m_oldPosition - a_referencePosition;
				t_offset = glm::vec3{ glm::mat4_cast(a_angularMove)
					* glm::vec4{t_offset, 0.0f} } - t_offset;
				a_linearMove += t_offset;

				t_linearMove += a_linearMove;
				t_angularMove = a_angularMove * t_angularMove;

				if (t_hierarchy != nullptr
					&& (!isNullWithEpsilon(t_linearMove)
					|| !isNullWithEpsilon(t_angularMove)))
				{
					for (auto t_entityId : t_hierarchy->m_children)
					{
						if (auto const t_childEntity = m_entities.find(t_entityId))
						{
							process(*t_childEntity
								, t_linearMove
								, t_angularMove
								, t_hierarchyTransform.m_oldPosition);
						}
					}
				}

				t_transform.m_position += a_linearMove;
				t_transform.m_rotation = a_angularMove * t_transform.m_rotation;
			}
		};
	}
}
