#pragma once

#include <vob/aoe/ecs/component_list.h>

#include <vob/misc/std/polymorphic_ptr_util.h>

#include <typeindex>
#include <unordered_map>


namespace vob::aoecs
{
	namespace detail
	{
		class component_list_factory_base
		{
		public:
			virtual mistd::polymorphic_ptr<component_list_base> create() const = 0;
		};

		template <typename TComponent>
		class component_list_factory final : public component_list_factory_base
		{
		public:
			mistd::polymorphic_ptr<component_list_base> create() const override
			{
				return mistd::pmr::polymorphic_ptr_util::make<component_list<TComponent>>();
			}
		};
	}

	class component_list_factory
	{
	public:
		template <typename TComponent>
		void register_component()
		{
			m_factories.emplace(
				typeid(TComponent),
				mistd::pmr::polymorphic_ptr_util::make<detail::component_list_factory<TComponent>>()
			);
		}

		mistd::polymorphic_ptr<component_list_base> create(std::type_index a_type) const
		{
			auto const it = m_factories.find(a_type);
			if (it == m_factories.end())
			{
				return nullptr;
			}

			return it->second->create();
		}

	private:
		std::pmr::unordered_map<
			std::type_index,
			mistd::polymorphic_ptr<detail::component_list_factory_base>
		> m_factories;
	};
}
