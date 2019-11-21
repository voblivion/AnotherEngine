#pragma once

#include <algorithm>
#include <cassert>
#include <typeindex>
#include <unordered_set>

#include <vob/sta/memory.h>
#include <vob/sta/vector_map.h>
#include <vob/sta/vector_set.h>

#include <vob/aoe/api.h>
#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/core/visitor/Aggregate.h>
#include <vob/aoe/core/visitor/Utils.h>
#include "vob/aoe/core/type/Clone.h"

namespace vob::aoe::ecs
{
	class CloneAComponentFactory
	{
	public:
		explicit CloneAComponentFactory(type::CloneCopier const& a_cloneCopier)
			: m_cloneCopier{ a_cloneCopier }
		{}

		type::Clone<AComponent> operator()() const
		{
			return type::Clone<AComponent>{ m_cloneCopier };
		}

	private:
		type::CloneCopier const& m_cloneCopier;
	};

	class VOB_AOE_API ComponentManager
		: public vis::Aggregate<ComponentManager, type::ADynamicType>
	{
	public:
		// Constructors
		explicit ComponentManager(
			type::CloneCopier const& a_cloneCopier
			, std::pmr::memory_resource* a_resource =
				std::pmr::get_default_resource())
			: m_components{ std::pmr::polymorphic_allocator<std::byte>{ a_resource } }
			, m_cloneCopier{ a_cloneCopier }
		{}

		template <typename ComponentType>
		bool hasComponent() const
		{
			return hasComponent(typeid(ComponentType));
		}

		bool hasComponent(std::type_index const a_typeIndex) const
		{
			return std::find_if(m_components.begin(), m_components.end()
				, [a_typeIndex](auto const& a_component)
			{
				return std::type_index{ typeid(*a_component) } == a_typeIndex;
			}) != m_components.end();
		}

		template <typename ComponentType, typename... Args>
		ComponentType& addComponent(Args&&... a_args)
		{
			auto const t_typeIndex = std::type_index{ typeid(ComponentType) };
			assert(!hasComponent(t_typeIndex));
			auto t_component = sta::allocate_polymorphic<ComponentType>(
				m_components.get_allocator()
				, std::forward<Args>(a_args)...
			);
			auto& t_result = *t_component;
			m_components.emplace(type::Clone<AComponent>{
				m_cloneCopier
				, std::move(t_component) });
			return t_result;
		}

		void addComponent(type::Clone<AComponent> a_component)
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
			auto const t_it = std::find_if(m_components.begin()
				, m_components.end(), [](auto const& a_component)
			{
				return typeid(*a_component) == typeid(std::remove_const_t<ComponentType>);
			});

			if (t_it != m_components.end())
			{
				return static_cast<ComponentType*>(t_it->get());
			}
			return nullptr;
		}

		std::pmr::polymorphic_allocator<std::byte> getAllocator() const;

	private:
		// Attributes
		struct PolymorphicComponentEqual
		{
			bool operator()(
				type::Clone<AComponent> const& a_lhs, type::Clone<AComponent> const& a_rhs
			) const
			{
				auto& lhs = *a_lhs;
				auto& rhs = *a_rhs;
				return typeid(lhs) == typeid(rhs);
			}
		};

		sta::pmr::vector_set<type::Clone<AComponent>, PolymorphicComponentEqual> m_components;
		std::reference_wrapper<type::CloneCopier const> m_cloneCopier;

		// Methods
		friend class vis::Aggregate<ComponentManager, type::ADynamicType>;
		template <typename VisitorType, typename ThisType>
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
			a_visitor.visit(vis::makeContainerHolder(a_this.m_components
				, CloneAComponentFactory{ a_this.m_cloneCopier }));
		}
	};
}
