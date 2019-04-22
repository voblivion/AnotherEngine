#include <aoe/core/ComponentManager.h>


namespace aoe
{
	namespace core
	{
		// Public
		ComponentManager::ComponentManager(
			ComponentManager const& a_componentManager)
			: m_components{ a_componentManager.m_components.get_allocator() }
		{
			*this = a_componentManager;
		}

		ComponentManager& ComponentManager::operator=(
			ComponentManager const& a_componentManager)
		{
			auto const t_allocator = m_components.get_allocator();
			m_components.reserve(a_componentManager.m_components.size());
			for (auto const& t_component : a_componentManager.m_components)
			{
				m_components.emplace(t_component.first
					,t_component.second->clone(t_allocator.resource()));
			}
			return *this;
		}
	}
}