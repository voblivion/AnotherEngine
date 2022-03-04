#pragma once

#include <vob/aoe/ecs/WorldDataProvider.h>
#include <vob/aoe/common/map/Hierarchycomponent.h>


namespace vob::aoe::common
{
	struct HierarchySystem
	{
		using Components = aoecs::ComponentTypeList<HierarchyComponent>;

		explicit HierarchySystem(aoecs::WorldDataProvider& a_wdp)
			: m_systemUnspawnManager{ a_wdp.getUnspawnManager() }
			, m_entities{ a_wdp.getEntityViewList(*this, Components{}) }
		{}

		void onEntityAdded(aoecs::entity& a_entity) const
		{
			auto const hierarchy = a_entity.getComponent<HierarchyComponent>();
			auto const parent = m_entities.find(hierarchy->m_parent);
			ignorable_assert(!hierarchy->m_parent.is_valid() || parent != nullptr);
			if(parent != nullptr)
			{
				auto& parentHierarchy = parent->getComponent<HierarchyComponent>();
				parentHierarchy.m_children.emplace_back(a_entity.get_id());
			}
			else
			{
				hierarchy->m_parent.reset();
			}
		}

		void onEntityRemoved(aoecs::entity& a_entity) const
		{
			auto& hierarchy = *a_entity.getComponent<HierarchyComponent>();

			// Remove from parent hierarchy
			if (hierarchy.m_parent.is_valid())
			{
				auto const parent = m_entities.find(hierarchy.m_parent);
				assert(parent != nullptr);
				auto& parentHierarchy = parent->getComponent<HierarchyComponent>();
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
				auto& childHierarchy = childEntity->getComponent<HierarchyComponent>();
				childHierarchy.m_parent.reset();
				m_systemUnspawnManager.unspawn(childEntity->getId());
			}
			hierarchy.m_children.clear();
		}

		void update() const {}

	private:
		aoecs::SystemUnspawnManager& m_systemUnspawnManager;
		aoecs::EntityViewList<HierarchyComponent> const& m_entities;
	};
}
