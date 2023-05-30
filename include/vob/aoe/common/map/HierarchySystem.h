#pragma once

#include <vob/aoe/ecs/world_data_provider.h>
#include <vob/aoe/ecs/entity_map_observer_list_ref.h>
#include <vob/aoe/common/map/Hierarchycomponent.h>

#include <vob/misc/std/ignorable_assert.h>


namespace vob::aoe::common
{
	struct HierarchySystem
	{
		explicit HierarchySystem(aoecs::world_data_provider& a_wdp)
			: m_unspawnManager{ a_wdp.get_despawner() }
			, m_entities{ a_wdp }
		{
			a_wdp.observe_spawns(*this);
			a_wdp.observe_despawns(*this);
		}

		void on_spawn(aoecs::entity_list::entity_view& a_entity) const
		{
			auto const hierarchy = a_entity.get_component<HierarchyComponent>();
			auto const parent = m_entities.find(hierarchy->m_parent);
			ignorable_assert(hierarchy->m_parent == aoecs::k_invalidEntityId || parent != m_entities.end());
			if(parent != m_entities.end())
			{
				auto& parentHierarchy = parent->get<HierarchyComponent>();
				parentHierarchy.m_children.emplace_back(a_entity.get_id());
			}
			else
			{
				hierarchy->m_parent = aoecs::k_invalidEntityId;
			}
		}

		void on_despawn(aoecs::entity_list::entity_view& a_entity) const
		{
			auto& hierarchy = *a_entity.get_component<HierarchyComponent>();

			// Remove from parent hierarchy
			if (hierarchy.m_parent != aoecs::k_invalidEntityId)
			{
				auto const parent = m_entities.find(hierarchy.m_parent);
				assert(parent != m_entities.end());
				auto& parentHierarchy = parent->get<HierarchyComponent>();
				auto const it = std::find(
					parentHierarchy.m_children.begin()
					, parentHierarchy.m_children.end()
					, a_entity.get_id()
				);
				ignorable_assert(it != parentHierarchy.m_children.end());
				if (it != parentHierarchy.m_children.end())
				{
					parentHierarchy.m_children.erase(it);
				}
			}

			// Request children to be removed
			// TODO: remove same frame ?
			for (auto const& childHandle : hierarchy.m_children)
			{
				auto const childEntity = m_entities.find(childHandle);
				assert(childEntity != m_entities.end());
				auto& childHierarchy = childEntity->get<HierarchyComponent>();
				childHierarchy.m_parent = aoecs::k_invalidEntityId;
				m_unspawnManager.despawn(childEntity->get_id());
			}
			hierarchy.m_children.clear();
		}

		void update() const {}

	private:
		aoecs::entity_manager::despawner& m_unspawnManager;
		aoecs::entity_map_observer_list_ref<HierarchyComponent> m_entities;
	};
}
