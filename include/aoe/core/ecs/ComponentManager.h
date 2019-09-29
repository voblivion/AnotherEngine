#pragma once

#include <cassert>
#include <typeindex>
#include <unordered_set>

#include <aoe/core/Export.h>
#include <aoe/core/ecs/Component.h>
#include <aoe/core/standard/Allocator.h>
#include <aoe/core/standard/Memory.h>
#include <aoe/core/standard/VectorMap.h>
#include <aoe/core/visitor/Aggregate.h>
#include <aoe/core/standard/PolymorphicEqual.h>
#include "aoe/core/standard/VectorSet.h"
#include "aoe/core/type/Clone.h"

namespace aoe
{
	namespace ecs
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

		class ComponentManager
			: public vis::Aggregate<ComponentManager, sta::ADynamicType>
		{
		public:
			// Constructors
			AOE_CORE_API explicit ComponentManager(
				type::CloneCopier const& a_cloneCopier
				, std::pmr::memory_resource* a_resource =
					std::pmr::get_default_resource())
				: m_components{ sta::Allocator<std::byte>{ a_resource } }
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
				auto t_component = sta::allocatePolymorphicWith<ComponentType>(
					m_components.get_allocator().resource()
					, std::forward<Args>(a_args)...);
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
					return typeid(*a_component) == typeid(ComponentType);
				});

				if (t_it != m_components.end())
				{
					return static_cast<ComponentType*>(t_it->get());
				}
				return nullptr;
			}

			AOE_CORE_API sta::Allocator<std::byte> getAllocator() const;

		private:
			// Attributes
			sta::pmr::VectorSet<type::Clone<AComponent>
				, sta::PolymorphicEqual<type::Clone<AComponent>>
			> m_components;
			std::reference_wrapper<type::CloneCopier const> m_cloneCopier;

			// Methods
			friend class vis::Aggregate<ComponentManager, sta::ADynamicType>;
			template <typename VisitorType, typename ThisType>
			static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
			{
				a_visitor.visit(vis::makeContainerHolder(a_this.m_components
					, CloneAComponentFactory{ a_this.m_cloneCopier }));
			}
		};
	}
}
