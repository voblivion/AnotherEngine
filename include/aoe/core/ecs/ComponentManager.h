#pragma once

#include <cassert>
#include <typeindex>

#include <aoe/core/Export.h>
#include <aoe/core/ecs/Component.h>
#include <aoe/core/standard/Allocator.h>
#include <aoe/core/standard/Memory.h>
#include <aoe/core/standard/VectorMap.h>
#include <aoe/core/visitor/Standard.h>

namespace aoe
{
	namespace ecs
	{
		class ComponentManager
			: public sta::ADynamicType
		{
		public:
			// Constructors
			AOE_CORE_API explicit ComponentManager(
				sta::Allocator<std::byte> const& a_allocator)
			{}

			AOE_CORE_API ComponentManager(ComponentManager&&) = default;

			AOE_CORE_API ComponentManager(ComponentManager const& a_componentManager);

			AOE_CORE_API virtual ~ComponentManager() = default;

			// Methods
			template <typename VisitorType, std::enable_if_t<
				VisitorType::accessType == visitor::AccessType::Reader>* = nullptr>
				void accept(VisitorType& a_visitor)
			{
				visitor::SizeTag t_size{ m_components.size() };
				a_visitor.visit(t_size);
				std::size_t t_index{ 0 };
				for (auto t_pair : m_components)
				{
					a_visitor.visit(t_index++, t_pair.second);
				}
			}

			template <typename VisitorType, std::enable_if_t<
				VisitorType::accessType == visitor::AccessType::Writer>* = nullptr>
			void accept(VisitorType& a_visitor)
			{
				visitor::SizeTag t_size{};
				a_visitor.visit(t_size);
				m_components.reserve(t_size.m_size + m_components.size());
				for (std::size_t t_index = 0; t_index < t_size.m_size; ++t_index)
				{
					sta::PolymorphicPtr<AComponent> t_component;
					a_visitor.visit(t_index, t_component);
					if(t_component != nullptr)
					{
						auto const t_typeId = std::type_index{ typeid(*t_component) };

						m_components.emplace(t_typeId
							, std::move(t_component));
					}
				}
			}

			template <typename ComponentType>
			bool hasComponent() const
			{
				return hasComponent(typeid(ComponentType));
			}

			bool hasComponent(std::type_index const a_typeIndex) const
			{
				return m_components.find(a_typeIndex) != m_components.end();
			}

			template <typename ComponentType>
			void addComponent(ComponentType a_component = {})
			{
				assert(!hasComponent<ComponentType>());
				auto const t_allocator = m_components.get_allocator();
				m_components.emplace(std::type_index(typeid(ComponentType))
					, sta::allocatePolymorphic<ComponentType>(t_allocator
						, std::move(a_component)));
			}

			void addComponent(AComponent const& a_component)
			{
				assert(!hasComponent(typeid(a_component)));
				auto const t_allocator = m_components.get_allocator();
				m_components.emplace(typeid(a_component)
					, a_component.clone(t_allocator.resource()));
			}

			void setComponent(AComponent const& a_component)
			{
				auto t_component = m_components.find(typeid(a_component));
				if (t_component != m_components.end())
				{
					t_component->second->copyFrom(a_component);
				}
				else
				{
					addComponent(a_component);
				}
			}

			auto const& getComponents() const
			{
				return m_components;
			}

			template <typename ComponentType>
			ComponentType* getComponent() const
			{
				auto const t_it = m_components.find(typeid(ComponentType));
				if (t_it != m_components.end())
				{
					return static_cast<ComponentType*>(t_it->second.get());
				}
				return nullptr;
			}

			AOE_CORE_API sta::Allocator<std::byte> getAllocator() const;

			// Operators
			ComponentManager& operator=(ComponentManager&&) = default;
			AOE_CORE_API ComponentManager& operator=(
				ComponentManager const& a_componentManager);

		private:
			// Attributes
			sta::pmr::VectorMap<std::type_index
				, sta::PolymorphicPtr<AComponent>> m_components;
		};
	}
}
