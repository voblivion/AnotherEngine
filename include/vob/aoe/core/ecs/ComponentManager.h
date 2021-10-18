#pragma once

#include <algorithm>
#include <cassert>
#include <typeindex>
#include <unordered_set>

#include <vob/sta/vector_map.h>
#include <vob/sta/vector_set.h>

#include <vob/aoe/api.h>
#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/core/visitor/Utils.h>
#include "vob/aoe/core/type/Clone.h"
#include <vob/aoe/core/visitor/Traits.h>


namespace vob::aoe::ecs
{
	class CloneAComponentFactory
	{
	public:
		explicit CloneAComponentFactory(type::Cloner<> const& a_cloner)
			: m_cloner{ a_cloner }
		{}

		type::Cloneable<AComponent> operator()() const
		{
			return type::Cloneable<AComponent>{ m_cloner };
		}

	private:
		type::Cloner<> const& m_cloner;
	};

	class VOB_AOE_API ComponentManager
		: public vis::Visitable<ComponentManager, type::ADynamicType>
	{
	public:
		// Constructors
		explicit ComponentManager(type::Cloner<> const& a_cloner)
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
			type::Cloneable<AComponent> component{ m_cloner };
			auto& result = component.init<ComponentType>(std::forward<Args>(a_args)...);
			m_components.emplace(std::move(component));
			return result;
		}

		void addComponent(type::Cloneable<AComponent> a_component)
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
		ComponentType* getComponent() const
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

        template <typename VisitorType, typename Self>
        static void accept(VisitorType& a_visitor, Self& a_this)
        {
            a_visitor.visit(vis::makeContainerHolder(
                a_this.m_components
                , ecs::CloneAComponentFactory{ a_this.m_cloner }
            ));
        }

	private:
		// Attributes
		struct PolymorphicComponentEqual
		{
			bool operator()(
				type::Cloneable<AComponent> const& a_lhs
				, type::Cloneable<AComponent> const& a_rhs
			) const
			{
				auto& lhs = *a_lhs;
				auto& rhs = *a_rhs;
				return typeid(lhs) == typeid(rhs);
			}
		};

		sta::vector_set<type::Cloneable<AComponent>, PolymorphicComponentEqual> m_components;
		std::reference_wrapper<type::Cloner<> const> m_cloner;
	};
}
