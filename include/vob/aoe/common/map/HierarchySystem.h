#pragma once

#include <vob/aoe/ecs/world_data_provider.h>
#include <vob/aoe/common/map/Hierarchycomponent.h>

#include <vob/misc/std/ignorable_assert.h>


namespace vob::aoe::common
{
	struct HierarchySystem
	{
		using Components = aoecs::ComponentTypeList<HierarchyComponent>;

		explicit HierarchySystem(aoecs::world_data_provider& a_wdp)
			: m_unspawnManager{ a_wdp.get_old_unspawn_manager() }
			, m_entities{ a_wdp.get_old_entity_view_list(*this, Components{}) }
		{}

		void on_entity_added(_aoecs::entity& a_entity) const
		{
			auto const hierarchy = a_entity.get_component<HierarchyComponent>();
			auto const parent = m_entities.find(hierarchy->m_parent);
			ignorable_assert(!hierarchy->m_parent.is_valid() || parent != nullptr);
			if(parent != nullptr)
			{
				auto& parentHierarchy = parent->get_component<HierarchyComponent>();
				parentHierarchy.m_children.emplace_back(a_entity.get_id());
			}
			else
			{
				hierarchy->m_parent.reset();
			}
		}

		void on_entity_removed(_aoecs::entity& a_entity) const
		{
			auto& hierarchy = *a_entity.get_component<HierarchyComponent>();

			// Remove from parent hierarchy
			if (hierarchy.m_parent.is_valid())
			{
				auto const parent = m_entities.find(hierarchy.m_parent);
				assert(parent != nullptr);
				auto& parentHierarchy = parent->get_component<HierarchyComponent>();
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
				assert(childEntity != nullptr);
				auto& childHierarchy = childEntity->get_component<HierarchyComponent>();
				childHierarchy.m_parent.reset();
				m_unspawnManager.unspawn(childEntity->get_id());
			}
			hierarchy.m_children.clear();
		}

		void update() const {}

	private:
		_aoecs::unspawn_manager& m_unspawnManager;
		_aoecs::entity_view_list<HierarchyComponent> const& m_entities;
	};
}
