#pragma once

#include <vob/aoe/api.h>
#include <vob/aoe/ecs/Component.h>
#include <vob/aoe/core/visitor/Utils.h>
#include <vob/aoe/core/visitor/Traits.h>

#include <vob/aoe/core/type/ADynamicType.h>

#include <vob/misc/type/clone.h>
#include <vob/misc/std/vector_map.h>
#include <vob/misc/std/vector_set.h>

#include <algorithm>
#include <cassert>
#include <typeindex>
#include <unordered_set>


namespace vob::aoe::aoecs
{
	class CloneAComponentFactory
	{
	public:
		explicit CloneAComponentFactory(type::dynamic_type_clone_copier const& a_cloner)
			: m_cloner{ a_cloner }
		{}

		type::dynamic_type_clone<AComponent> operator()() const
		{
			return type::dynamic_type_clone<AComponent>{ m_cloner };
		}

	private:
		type::dynamic_type_clone_copier const& m_cloner;
	};

	class VOB_AOE_API ComponentManager
		: public type::ADynamicType
	{
	public:
		// Constructors
		explicit ComponentManager(type::dynamic_type_clone_copier const& a_cloner)
			: m_cloner{ a_cloner }
		{}

		template <typename ComponentType>
		bool hasComponent() const
		{
			return hasComponent(typeid(ComponentType));
		}

		bool hasComponent(std::type_index const a_typeIndex) const
		{
			return std::find_if(
				m_components.begin()
				, m_components.end()
				, [a_typeIndex](auto const& a_component)
				{
					return std::type_index{ typeid(*a_component) } == a_typeIndex;
				}
			) != m_components.end();
		}

		template <typename ComponentType, typename... Args>
		ComponentType& addComponent(Args&&... a_args)
		{
			auto const t_typeIndex = std::type_index{ typeid(ComponentType) };
			assert(!hasComponent(t_typeIndex));
			type::dynamic_type_clone<AComponent> component{ m_cloner };
			auto& result = component.init<ComponentType>(std::forward<Args>(a_args)...);
			m_components.emplace(std::move(component));
			return result;
		}

		void addComponent(type::dynamic_type_clone<AComponent> a_component)
		{
			auto const t_typeIndex = std::type_index{ typeid(a_component) };
			assert(!hasComponent(t_typeIndex));
			m_components.emplace(std::move(a_component));
		}

		auto const& getComponents() const
		{
			return m_components;
		}

		template <typename ComponentType>
		ComponentType const* getComponent() const
		{
			auto const t_it = std::find_if(
				m_components.begin()
				, m_components.end()
				, [](auto const& a_component)
				{
					return typeid(*a_component) == typeid(std::remove_const_t<ComponentType>);
				}
			);

			if (t_it != m_components.end())
			{
				return static_cast<ComponentType const*>(t_it->get());
			}
			return nullptr;
		}

		template <typename ComponentType>
		ComponentType* getComponent()
		{
			auto const t_it = std::find_if(
				m_components.begin()
				, m_components.end()
				, [](auto const& a_component)
				{
					return typeid(*a_component) == typeid(std::remove_const_t<ComponentType>);
				}
			);

			if (t_it != m_components.end())
			{
				return static_cast<ComponentType*>(t_it->get());
			}
			return nullptr;
		}

        template <typename VisitorType>
        void accept(VisitorType& a_visitor)
        {
            a_visitor.visit(vis::makeContainerHolder(
                m_components
                , aoecs::CloneAComponentFactory{ m_cloner }
            ));
        }

		template <typename VisitorType>
		void accept(VisitorType& a_visitor) const
		{
			a_visitor.visit(m_components);
		}

	private:
		// Attributes
		struct PolymorphicComponentEqual
		{
			bool operator()(
				type::dynamic_type_clone<AComponent> const& a_lhs
				, type::dynamic_type_clone<AComponent> const& a_rhs
			) const
			{
				auto& lhs = *a_lhs;
				auto& rhs = *a_rhs;
				return typeid(lhs) == typeid(rhs);
			}
		};

		mistd::vector_set<type::dynamic_type_clone<AComponent>, PolymorphicComponentEqual> m_components;
		std::reference_wrapper<type::dynamic_type_clone_copier const> m_cloner;
	};
}
