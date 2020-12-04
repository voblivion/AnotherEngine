#pragma once
#include <vob/aoe/core/ecs/WorldDataProvider.h>
#include <vob/aoe/common/map/HierarchyComponent.h>
#include <vob/aoe/common/space/TransformComponent.h>
#include <vob/aoe/common/space/LocalTransformComponent.h>

namespace vob::aoe::common
{
	class LocalTransformSystem final
	{
	public:
		using Components = ecs::ComponentTypeList<
			HierarchyComponent const
			, TransformComponent
			, LocalTransformComponent*
		>;

		explicit LocalTransformSystem(ecs::WorldDataProvider& a_worldDataProvider)
			: m_entities{ a_worldDataProvider.getEntityList(*this, Components{}) }
		{}

		void update() const
		{
			for (auto const& entity : m_entities)
			{
				auto const hierarchy = entity.getComponent<HierarchyComponent>();
				if (hierarchy.m_parent.isValid())
				{
					continue;
				}

				updateChildrenTransform(entity);
			}
		}

	private:
		// Attributes
		using EntityList = ecs::EntityList<
			HierarchyComponent const
			, TransformComponent
			, LocalTransformComponent*
		>;
		using Entity = EntityList::EntityType;

		EntityList const& m_entities;

		// Methods
		void updateChildrenTransform(Entity const& a_entity) const
		{
			auto const& hierarchy = a_entity.getComponent<HierarchyComponent>();
			auto const& transform = a_entity.getComponent<TransformComponent>();
			for (auto const& childHandle : hierarchy.m_children)
			{
				auto const& childEntity = m_entities.find(childHandle);
				if (childEntity != nullptr)
				{
					updateTransform(transform.m_matrix, *childEntity);
				}
			}
		}

		void updateTransform(glm::mat4 const& a_parentMatrix, Entity const& a_entity) const
		{
			auto& transform = a_entity.getComponent<TransformComponent>();
			auto const localTransform = a_entity.getComponent<LocalTransformComponent>();
			if (localTransform != nullptr)
			{
				transform.m_matrix = a_parentMatrix * localTransform->m_matrix;
			}

			updateChildrenTransform(a_entity);
		}
	};
}
