#pragma once

#include <vob/aoe/core/ecs/WorldDataProvider.h>
#include <vob/aoe/common/map/HierarchyComponent.h>


namespace vob::aoe::common
{
	struct HierarchySystem
	{
		using Components = ecs::ComponentTypeList<HierarchyComponent>;

		explicit HierarchySystem(ecs::WorldDataProvider& a_wdp)
			: m_systemUnspawnManager{ a_wdp.getUnspawnManager() }
			, m_entities{ a_wdp.getEntityViewList(*this, Components{}) }
		{}

		void onEntityAdded(ecs::Entity const& a_entity) const
		{
			auto const hierarchy = a_entity.getComponent<HierarchyComponent>();
			auto const parent = m_entities.find(hierarchy->m_parent);
			ignorable_assert(!hierarchy->m_parent.isValid() || parent != nullptr);
			if(parent != nullptr)
			{
				auto& parentHierarchy = parent->getComponent<HierarchyComponent>();
				parentHierarchy.m_children.emplace_back(a_entity);
			}
			else
			{
				hierarchy->m_parent.reset();
			}
		}

		void onEntityRemoved(ecs::Entity const& a_entity) const
		{
			auto& hierarchy = *a_entity.getComponent<HierarchyComponent>();

			// Remove from parent hierarchy
			if (hierarchy.m_parent.isValid())
			{
				auto const parent = m_entities.find(hierarchy.m_parent);
				assert(parent != nullptr);
				auto& parentHierarchy = parent->getComponent<HierarchyComponent>();
				auto const it = std::find(
					parentHierarchy.m_children.begin()
					, parentHierarchy.m_children.end()
					, ecs::EntityHandle{ a_entity }
				);
				ignorable_assert(it != parentHierarchy.m_children.end());
				if (it != parentHierarchy.m_children.end())
				{
					parentHierarchy.m_children.erase(it);
				}
			}

			// Request children to be removed
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

		void update() {}

	private:
		ecs::SystemUnspawnManager& m_systemUnspawnManager;
		ecs::EntityViewList<HierarchyComponent> const& m_entities;
	};
}
