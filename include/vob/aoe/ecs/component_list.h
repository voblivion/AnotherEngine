#pragma once

#include <vob/aoe/ecs/component_set.h>


namespace vob::aoecs
{
	class component_list_base
	{
	public:
		virtual void add_from(component_set const& a_componentSet) = 0;
		virtual void swap_remove_at(std::size_t a_index) = 0;
	};

	template <typename TComponent>
	class component_list final : public component_list_base
	{
	public:
		void add_from(component_set const& a_componentSet) override
		{
			auto const component = a_componentSet.find<TComponent>();
			assert(component != nullptr);
			m_components.emplace_back(*component);
		}

		void swap_remove_at(std::size_t a_index) override
		{
			std::swap(m_components[a_index], m_components.back());
			m_components.pop_back();
		}

		TComponent& at(std::size_t a_index)
		{
			assert(a_index < m_components.size());
			return m_components[a_index];
		}

		TComponent const& at(std::size_t a_index) const
		{
			assert(a_index < m_components.size());
			return m_components[a_index];
		}

	private:
		std::pmr::vector<TComponent> m_components;
	};
}
