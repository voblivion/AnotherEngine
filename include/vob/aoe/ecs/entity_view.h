#pragma once

#include <deque>

#include <vob/aoe/ecs/entity.h>
#include <vob/aoe/ecs/entity_id.h>


namespace vob::aoecs
{
	namespace detail
	{
		template <typename... ComponentTypes>
		class SystemEntityViewImpl;

		template <>
		class SystemEntityViewImpl<>
		{
		public:
			explicit SystemEntityViewImpl(entity&) {}

			static void get() {}
		};

		template <typename ComponentType>
		struct Component {};

		template <typename ComponentType, typename... ComponentTypes>
		class SystemEntityViewImpl<ComponentType, ComponentTypes...>
			: public SystemEntityViewImpl<ComponentTypes...>
		{
			using Base = SystemEntityViewImpl<ComponentTypes...>;
		public:
			// Constructors
			explicit SystemEntityViewImpl(entity& a_entity)
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
		class SystemEntityViewImpl<ComponentType*, ComponentTypes...>
			: public SystemEntityViewImpl<ComponentTypes...>
		{
			using Base = SystemEntityViewImpl<ComponentTypes...>;
		public:
			// Constructors
			explicit SystemEntityViewImpl(entity& a_entity)
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
		class SystemEntityViewImpl<ComponentType const, ComponentTypes...>
			: public SystemEntityViewImpl<ComponentTypes...>
		{
			using Base = SystemEntityViewImpl<ComponentTypes...>;
		public:
			// Constructors
			explicit SystemEntityViewImpl(entity& a_entity)
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
		class SystemEntityViewImpl<ComponentType const*, ComponentTypes...>
			: public SystemEntityViewImpl<ComponentTypes...>
		{
			using Base = SystemEntityViewImpl<ComponentTypes...>;
		public:
			// Constructors
			explicit SystemEntityViewImpl(entity& a_entity)
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
	class EntityView
	{
	public:
		// Constructors
		explicit EntityView(entity& a_entity)
			: m_id{ a_entity.get_id() }
			, m_impl{ a_entity }
#ifndef NDEBUG
			, m_entity{ &a_entity }
#endif
		{}

		// Methods
		entity_id getId() const
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
		entity_id m_id;
		detail::SystemEntityViewImpl<ComponentTypes...> m_impl;
#ifndef NDEBUG
		entity* m_entity = nullptr;
#endif
	};
}
