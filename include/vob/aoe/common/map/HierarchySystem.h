#pragma once

#include <vob/aoe/core/ecs/WorldDataProvider.h>
#include <vob/aoe/common/map/HierarchyComponent.h>


namespace vob::aoe::common
{
	struct HierarchySystem
	{
		using Components = ecs::ComponentTypeList<HierarchyComponent>;

		explicit HierarchySystem(ecs::WorldDataProvider& a_wdp)
			: m_entities{ a_wdp.getEntityList(*this, Components{}) }
		{}

		void onEntityAdded(ecs::Entity const& a_entity) const
		{
			auto const t_hierarchy = a_entity.getComponent<HierarchyComponent>();
			auto const t_parent = m_entities.find(t_hierarchy->m_parent);
			ignorable_assert(!t_hierarchy->m_parent.isValid() || t_parent != nullptr);
			if(t_parent != nullptr)
			{
				auto& t_parentHierarchy = t_parent->getComponent<HierarchyComponent>();
				t_parentHierarchy.m_children.emplace_back(
					a_entity
				);
			}
			else
			{
				t_hierarchy->m_parent.reset();
			}
		}

		void update() {}

	private:
		ecs::EntityList<HierarchyComponent> const& m_entities;
	};
}
