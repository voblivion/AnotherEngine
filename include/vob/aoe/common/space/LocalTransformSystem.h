#pragma once
#include <vob/aoe/ecs/world_data_provider.h>
#include <vob/aoe/ecs/entity_map_observer_list_ref.h>
#include <vob/aoe/common/map/Hierarchycomponent.h>
#include <vob/aoe/common/space/Transformcomponent.h>
#include <vob/aoe/common/space/LocalTransformcomponent.h>

namespace vob::aoe::common
{
	class LocalTransformSystem final
	{
	public:
		explicit LocalTransformSystem(aoecs::world_data_provider& a_wdp)
			: m_entities{ a_wdp }
		{}

		void update() const
		{
			for (auto const& entity : m_entities)
			{
				auto const hierarchy = entity.get<HierarchyComponent>();
				if (hierarchy.m_parent != aoecs::k_invalidEntityId)
				{
					continue;
				}

				updateChildrenTransform(entity);
			}
		}

	private:
		// Attributes
		using entity_view = aoecs::entity_map_observer_list<HierarchyComponent const, TransformComponent, LocalTransformComponent*>::view;

		aoecs::entity_map_observer_list_ref<HierarchyComponent const, TransformComponent, LocalTransformComponent*> m_entities;

		// Methods
		void updateChildrenTransform(entity_view const& a_entity) const
		{
			auto const& hierarchy = a_entity.get<HierarchyComponent>();
			auto const& transform = a_entity.get<TransformComponent>();
			for (auto const& childHandle : hierarchy.m_children)
			{
				auto const childEntity = m_entities.find(childHandle);
				if (childEntity != m_entities.end())
				{
					updateTransform(transform.m_matrix, *childEntity);
				}
			}
		}

		void updateTransform(glm::mat4 const& a_parentMatrix, entity_view const& a_entity) const
		{
			auto& transform = a_entity.get<TransformComponent>();
			auto const localTransform = a_entity.get<LocalTransformComponent>();
			if (localTransform != nullptr)
			{
				transform.m_matrix = a_parentMatrix * localTransform->m_matrix;
			}

			updateChildrenTransform(a_entity);
		}
	};
}
