
#pragma once

#include <vob/aoe/api.h>
#include <vob/aoe/ecs/component_holder.h>

#include <vob/aoe/core/type/ADynamicType.h>

#include <vob/misc/type/clone.h>
#include <vob/misc/std/vector_map.h>
#include <vob/misc/std/vector_set.h>

#include <vob/misc/visitor/container.h>

#include <algorithm>
#include <cassert>
#include <typeindex>
#include <unordered_set>


namespace vob::aoecs
{
	class component_holder_clone_factory
	{
	public:
		explicit component_holder_clone_factory(component_holder_cloner const& a_cloner)
			: m_cloner{ a_cloner }
		{}

		basic_component_holder_clone operator()() const
		{
			return basic_component_holder_clone{ m_cloner };
		}

	private:
		component_holder_cloner const& m_cloner;
	};

	class VOB_AOE_API component_manager
		: public aoe::type::ADynamicType
	{
	public:
		// Constructors
		explicit component_manager(component_holder_cloner const& a_cloner)
			: m_cloner{ a_cloner }
		{}

		template <typename TComponent>
		bool has_component() const
		{
			return has_component(typeid(component_holder<TComponent>));
		}

		bool has_component(std::type_index const a_typeIndex) const
		{
			return std::find_if(
				m_components.begin()
				, m_components.end()
				, [a_typeIndex](auto const& a_componentHolderClone)
				{
					return std::type_index{ typeid(*a_componentHolderClone) } == a_typeIndex;
				}
			) != m_components.end();
		}

		template <typename TComponent, typename... Args>
		TComponent& addComponent(Args&&... a_args)
		{
			auto const t_typeIndex = std::type_index{ typeid(component_holder<TComponent>) };
			assert(!has_component(t_typeIndex));
			basic_component_holder_clone componentHolder{ m_cloner };
			auto& result = componentHolder.init<component_holder<TComponent>>(std::forward<Args>(a_args)...);
			m_components.emplace(std::move(componentHolder));
			return result.m_component;
		}

		void addComponent(basic_component_holder_clone a_componentHolderClone)
		{
			auto const t_typeIndex = std::type_index{ typeid(*a_componentHolderClone) };
			assert(!has_component(t_typeIndex));
			m_components.emplace(std::move(a_componentHolderClone));
		}

		auto const& getComponents() const
		{
			return m_components;
		}

		template <typename TComponent>
		TComponent const* get_component() const
		{
			auto const t_it = std::find_if(
				m_components.begin()
				, m_components.end()
				, [](auto const& a_componentHolderClone)
				{
					return typeid(*a_componentHolderClone) == typeid(component_holder<std::remove_const_t<TComponent>>);
				}
			);

			if (t_it != m_components.end())
			{
				return &(static_cast<component_holder<TComponent>*>(t_it->get())->m_component);
			}
			return nullptr;
		}

		template <typename TComponent>
		TComponent* get_component()
		{
			auto const t_it = std::find_if(
				m_components.begin()
				, m_components.end()
				, [](auto const& a_componentHolderClone)
				{
					return typeid(*a_componentHolderClone) == typeid(component_holder<std::remove_const_t<TComponent>>);
				}
			);

			if (t_it != m_components.end())
			{
				return &(static_cast<component_holder<TComponent>*>(t_it->get())->m_component);
			}
			return nullptr;
		}

        template <typename VisitorType>
        bool accept(VisitorType& a_visitor)
        {
            a_visitor.visit(misvi::ctn(
                m_components
                , aoecs::component_holder_clone_factory{ m_cloner }
            ));
			return true;
        }

		template <typename VisitorType>
		bool accept(VisitorType& a_visitor) const
		{
			a_visitor.visit(m_components);
			return true;
		}

	private:
		struct component_holder_clone_equal
		{
			bool operator()(
				basic_component_holder_clone const& a_lhs, basic_component_holder_clone const& a_rhs) const
			{
				return typeid(*a_lhs) == typeid(*a_rhs);
			}
		};

		mistd::vector_set<basic_component_holder_clone, component_holder_clone_equal> m_components;
		std::reference_wrapper<component_holder_cloner const> m_cloner;
	};
}
