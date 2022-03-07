#pragma once

#include <deque>

#include <vob/aoe/ecs/entity.h>
#include <vob/aoe/ecs/entity_id.h>


namespace vob::aoecs
{
	namespace detail
	{
		template <typename... TComponents>
		class entity_view_impl;

		template <>
		class entity_view_impl<>
		{
		public:
			explicit entity_view_impl(entity&) {}

			static void get() {}
		};

		template <typename TComponent>
		struct Component {};

		template <typename TComponent, typename... TComponents>
		class entity_view_impl<TComponent, TComponents...>
			: public entity_view_impl<TComponents...>
		{
			using base = entity_view_impl<TComponents...>;
		public:
			// Constructors
			explicit entity_view_impl(entity& a_entity)
				: base{ a_entity }
				, m_component{ *a_entity.get_component<TComponent>() }
			{}

			// Methods

			TComponent& get(Component<TComponent>) const
			{
				return m_component.get();
			}
			using base::get;

		private:
			// Attributes
			std::reference_wrapper<TComponent> m_component;
		};

		template <typename TComponent, typename... TComponents>
		class entity_view_impl<TComponent*, TComponents...>
			: public entity_view_impl<TComponents...>
		{
			using base = entity_view_impl<TComponents...>;
		public:
			// Constructors
			explicit entity_view_impl(entity& a_entity)
				: base{ a_entity }
				, m_component{ a_entity.get_component<TComponent>() }
			{}

			// Methods

			TComponent* get(Component<TComponent>) const
			{
				return m_component;
			}
			using base::get;

		private:
			// Attributes
			TComponent* m_component;
		};

		template <typename TComponent, typename... TComponents>
		class entity_view_impl<TComponent const, TComponents...>
			: public entity_view_impl<TComponents...>
		{
			using base = entity_view_impl<TComponents...>;
		public:
			// Constructors
			explicit entity_view_impl(entity& a_entity)
				: base{ a_entity }
				, m_component{ *a_entity.get_component<TComponent>() }
			{}

			// Methods

			TComponent const& get(Component<TComponent>) const
			{
				return m_component.get();
			}
			using base::get;

		private:
			// Attributes
			std::reference_wrapper<TComponent const> m_component;
		};

		template <typename TComponent, typename... TComponents>
		class entity_view_impl<TComponent const*, TComponents...>
			: public entity_view_impl<TComponents...>
		{
			using base = entity_view_impl<TComponents...>;
		public:
			// Constructors
			explicit entity_view_impl(entity& a_entity)
				: base{ a_entity }
				, m_component{ a_entity.get_component<TComponent>() }
			{}

			// Methods

			TComponent const* get(Component<TComponent>) const
			{
				return m_component;
			}
			using base::get;

		private:
			// Attributes
			TComponent const* m_component;
		};
	}

	template <typename... TComponents>
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

		template <typename TComponent>
		decltype(auto) get_component() const
		{
			return m_impl.get(detail::Component<TComponent>{});
		}

	private:
		// Attributes
		entity_id m_id;
		detail::entity_view_impl<TComponents...> m_impl;
#ifndef NDEBUG
		entity* m_entity = nullptr;
#endif
	};
}
