#pragma once
#include <deque>
#include <aoe/core/ecs/Entity.h>
#include <aoe/core/ecs/EntityId.h>


namespace aoe
{
	namespace ecs
	{
		namespace detail
		{
			template <typename... ComponentTypes>
			class SystemEntityImpl;

			template <>
			class SystemEntityImpl<>
			{
			public:
				explicit SystemEntityImpl(Entity const& a_entity) {}

				static void get() {}
			};

			template <typename ComponentType>
			struct Component {};

			template <typename ComponentType, typename... ComponentTypes>
			class SystemEntityImpl<ComponentType, ComponentTypes...>
				: public SystemEntityImpl<ComponentTypes...>
			{
				using Base = SystemEntityImpl<ComponentTypes...>;
			public:
				// Constructors
				explicit SystemEntityImpl(Entity const& a_entity)
					: Base{ a_entity }
					, m_component{ *a_entity.getComponent<ComponentType>() }
				{}

				// Methods

				ComponentType& get(Component<ComponentType>) const
				{
					return m_component.get();
				}
				using Base::get;

			private:
				// Attributes
				std::reference_wrapper<ComponentType> m_component;
			};

			template <typename ComponentType, typename... ComponentTypes>
			class SystemEntityImpl<ComponentType*, ComponentTypes...>
				: public SystemEntityImpl<ComponentTypes...>
			{
				using Base = SystemEntityImpl<ComponentTypes...>;
			public:
				// Constructors
				explicit SystemEntityImpl(Entity const& a_entity)
					: Base{ a_entity }
					, m_component{ a_entity.getComponent<ComponentType>() }
				{}

				// Methods

				ComponentType* get(Component<ComponentType>) const
				{
					return m_component;
				}
				using Base::get;

			private:
				// Attributes
				ComponentType* m_component;
			};

			template <typename ComponentType, typename... ComponentTypes>
			class SystemEntityImpl<ComponentType const, ComponentTypes...>
				: public SystemEntityImpl<ComponentTypes...>
			{
				using Base = SystemEntityImpl<ComponentTypes...>;
			public:
				// Constructors
				explicit SystemEntityImpl(Entity const& a_entity)
					: Base{ a_entity }
					, m_component{ *a_entity.getComponent<ComponentType>() }
				{}

				// Methods

				ComponentType const& get(Component<ComponentType>) const
				{
					return m_component.get();
				}
				using Base::get;

			private:
				// Attributes
				std::reference_wrapper<ComponentType const> m_component;
			};

			template <typename ComponentType, typename... ComponentTypes>
			class SystemEntityImpl<ComponentType const*, ComponentTypes...>
				: public SystemEntityImpl<ComponentTypes...>
			{
				using Base = SystemEntityImpl<ComponentTypes...>;
			public:
				// Constructors
				explicit SystemEntityImpl(Entity const& a_entity)
					: Base{ a_entity }
					, m_component{ a_entity.getComponent<ComponentType>() }
				{}

				// Methods

				ComponentType const* get(Component<ComponentType>) const
				{
					return m_component;
				}
				using Base::get;

			private:
				// Attributes
				ComponentType const* m_component;
			};
		}

		template <typename... ComponentTypes>
		class SystemEntity
		{
		public:
			// Constructors
			explicit SystemEntity(Entity const& a_entity)
				: m_id{ a_entity.getId() }
				, m_impl{ a_entity }
			{}

			// Methods
			EntityId getId() const
			{
				return m_id;
			}

			template <typename ComponentType>
			decltype(auto) getComponent() const
			{
				return m_impl.get(detail::Component<ComponentType>{});
			}

		private:
			// Attributes
			EntityId const m_id;
			detail::SystemEntityImpl<ComponentTypes...> m_impl;
		};
	}
}
