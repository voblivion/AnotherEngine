#pragma once

#include <cassert>
#include <typeindex>

#include <aoe/core/Export.h>
#include <aoe/core/ecs/Component.h>
#include <aoe/core/standard/Allocator.h>
#include <aoe/core/standard/Memory.h>
#include <aoe/core/standard/VectorMap.h>

namespace aoe
{
	namespace ecs
	{
		class ComponentManager
		{
		public:
			// Constructors
			AOE_CORE_API explicit ComponentManager(
				sta::Allocator<std::byte> const& a_allocator)
			{}

			AOE_CORE_API ComponentManager(ComponentManager&&) = default;

			AOE_CORE_API ComponentManager(ComponentManager const& a_componentManager);

			AOE_CORE_API ~ComponentManager() = default;

			// Methods
			template <typename VisitorType>
			void accept(VisitorType& a_visitor)
			{
				makeVisit(a_visitor, m_components);
			}

			template <typename ComponentType>
			bool hasComponent() const
			{
				auto const t_it = m_components.find(typeid(ComponentType));
				return t_it != m_components.end();
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
