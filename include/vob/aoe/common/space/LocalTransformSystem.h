#pragma once
#include <vob/aoe/ecs/WorldDataProvider.h>
#include <vob/aoe/common/map/Hierarchycomponent.h>
#include <vob/aoe/common/space/Transformcomponent.h>
#include <vob/aoe/common/space/LocalTransformcomponent.h>

namespace vob::aoe::common
{
	class LocalTransformSystem final
	{
	public:
		using Components = aoecs::ComponentTypeList<
			HierarchyComponent const
			, TransformComponent
			, LocalTransformComponent*
		>;

		explicit LocalTransformSystem(aoecs::WorldDataProvider& a_worldDataProvider)
			: m_entities{ a_worldDataProvider.getEntityViewList(*this, Components{}) }
		{}

		void update() const
		{
			for (auto const& entity : m_entities)
			{
				auto const hierarchy = entity.get_component<HierarchyComponent>();
				if (hierarchy.m_parent.is_valid())
				{
					continue;
				}

				updateChildrenTransform(entity);
			}
		}

	private:
		// Attributes
		using EntityViewList = aoecs::EntityViewList<
			HierarchyComponent const
			, TransformComponent
			, LocalTransformComponent*
		>;
		using EntityView = EntityViewList::EntityViewType;

		EntityViewList const& m_entities;

		// Methods
		void updateChildrenTransform(EntityView const& a_entity) const
		{
			auto const& hierarchy = a_entity.get_component<HierarchyComponent>();
			auto const& transform = a_entity.get_component<TransformComponent>();
			for (auto const& childHandle : hierarchy.m_children)
			{
				auto const& childEntity = m_entities.find(childHandle);
				if (childEntity != nullptr)
				{
					updateTransform(transform.m_matrix, *childEntity);
				}
			}
		}

		void updateTransform(glm::mat4 const& a_parentMatrix, EntityView const& a_entity) const
		{
			auto& transform = a_entity.get_component<TransformComponent>();
			auto const localTransform = a_entity.get_component<LocalTransformComponent>();
			if (localTransform != nullptr)
			{
				transform.m_matrix = a_parentMatrix * localTransform->m_matrix;
			}

			updateChildrenTransform(a_entity);
		}
	};
}
