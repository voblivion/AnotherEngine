#pragma once

#include <vob/aoe/engine/world_data.h>

#include <vob/misc/std/message_macros.h>


namespace vob::aoeng
{
	template <typename TComponent>
	class world_component_accessor
	{
	public:
		explicit world_component_accessor(entt::registry& a_entityRegistry, entt::entity a_worldEntity)
			: m_entityRegistry{ a_entityRegistry }
			, m_worldEntity{ a_worldEntity }
		{
		}

		TComponent& get() const
		{
			return m_entityRegistry.get().get<TComponent>(m_worldEntity);
		}

	private:
		std::reference_wrapper<entt::registry> m_entityRegistry;
		entt::entity m_worldEntity;
	};

	template <typename... TComponents>
	class registry_view_accessor
	{
	public:
		explicit registry_view_accessor(entt::registry& a_entityRegistry)
			: m_entityRegistry{ a_entityRegistry }
		{
		}

		template <typename... TExcludes>
		decltype(auto) get() const
		{
			return m_entityRegistry.get().view<TComponents...>(entt::exclude_t<TExcludes...>{});
		}

		decltype(auto) get() const
		{
			return m_entityRegistry.get().view<TComponents...>();
		}

	private:
		std::reference_wrapper<entt::registry> m_entityRegistry;
	};

	class world_data_provider
	{
#pragma message(VOB_MISTD_TODO "collect accesses for safe multithreading.")

	public:
		explicit world_data_provider(world_data& a_worldData)
			: m_worldData{ a_worldData }
		{
		}

		bool& get_should_stop()
		{
			return m_worldData.m_shouldStop;
		}

		template <typename TComponent>
		world_component_accessor<TComponent> get_world_component_accessor()
		{
			return world_component_accessor<TComponent>(m_worldData.m_entityRegistry, m_worldData.m_worldEntity);
		}

		entity_registry_query_queue& get_pending_entity_registry_queries()
		{
			return m_worldData.m_pendingEntityRegistryQueries;
		}

		template <typename... TComponents>
		registry_view_accessor<TComponents...> get_registry_view_accessor()
		{
			return registry_view_accessor<TComponents...>(m_worldData.m_entityRegistry);
		}

		template <typename TComponent, auto TCandidate, typename... TValues>
		void on_construct(TValues&&... a_values)
		{
			m_worldData.m_entityRegistry.on_construct<TComponent>().connect<TCandidate>(
				std::forward<TValues>(a_values)...);
		}

		template <typename TComponent, auto TCandidate, typename... TValues>
		void on_destroy(TValues&&... a_values)
		{
			m_worldData.m_entityRegistry.on_destroy<TComponent>().connect<TCandidate>(
				std::forward<TValues>(a_values)...);
		}


	private:
		world_data& m_worldData;
	};

	class should_stop_ref
	{
	public:
		explicit should_stop_ref(world_data_provider& a_wdp)
			: m_shouldStop{ a_wdp.get_should_stop() }
		{
		}

		void set(bool a_shouldStop) const
		{
			m_shouldStop.get() = a_shouldStop;
		}

		bool get() const
		{
			return m_shouldStop.get();
		}

		bool& operator*() const
		{
			return m_shouldStop.get();
		}

	private:
		std::reference_wrapper<bool> m_shouldStop;
	};

	template <typename TComponent>
	class world_component_ref
	{
	public:
		explicit world_component_ref(world_data_provider& a_wdp)
			: m_worldComponentAccessor{ a_wdp.get_world_component_accessor<TComponent>() }
		{
		}

		TComponent& get() const
		{
			return m_worldComponentAccessor.get();
		}

		TComponent& operator*() const
		{
			return get();
		}

		TComponent* operator->() const
		{
			return &get();
		}

	private:
		world_component_accessor<TComponent> m_worldComponentAccessor;
	};

	class pending_entity_registry_query_queue_ref
	{
	public:
		explicit pending_entity_registry_query_queue_ref(world_data_provider& a_wdp)
			: m_queries{ a_wdp.get_pending_entity_registry_queries() }
		{
		}

		void add(entity_registry_query a_query) const
		{
			m_queries.get().emplace_back(std::move(a_query));
		}

	private:
		std::reference_wrapper<entity_registry_query_queue> m_queries;
	};

	template <typename... TComponents>
	class registry_view_ref
	{
	public:
		explicit registry_view_ref(world_data_provider& a_wdp)
			: m_registryViewAccessor{ a_wdp.get_registry_view_accessor<TComponents...>() }
		{
		}

		template <typename... TExcludes>
		decltype(auto) get() const
		{
			return m_registryViewAccessor.get<TExcludes...>();
		}

		decltype(auto) get() const
		{
			return m_registryViewAccessor.get();
		}

		decltype(auto) operator*() const
		{
			return get();
		}

	private:
		registry_view_accessor<TComponents...> m_registryViewAccessor;
	};
}
