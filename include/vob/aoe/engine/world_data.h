#pragma once

#include <vob/aoe/api.h>

#include <vob/misc/std/message_macros.h>

#include <entt/entt.hpp>


namespace vob::aoeng
{
	using entity = entt::entity;
	using registry_allocator = std::allocator<entity>; // std::pmr::polymorphic_allocator<entity>;
	using registry = entt::basic_registry<entt::entity, registry_allocator>;
	
	using registry_query = std::function<void(registry&)>;
	using registry_query_queue = std::pmr::vector<registry_query>;
	using registry_query_allocator = std::pmr::polymorphic_allocator<registry_query>;

	class VOB_AOE_API world_data
	{
	public:
		explicit world_data(
			registry_allocator const& a_registryAllocator = {},
			registry_query_allocator const& a_entityRegistryQueryAllocator = {}
		);

		template <typename TComponent, typename... TArgs>
		decltype(auto) add_world_component(TArgs&&... a_args)
		{
			return m_registry.ctx().emplace<TComponent>(std::forward<TArgs>(a_args)...);
		}

		template <typename TComponent>
		decltype(auto) get_world_component()
		{
			return m_registry.ctx().get<TComponent>();
		}

		bool should_stop() const
		{
			return m_shouldStop;
		}

#pragma message(VOB_MISTD_TODO "those functions are needed for world init, but need a better pattern.")
#pragma region TMP
		registry& get_entity_registry()
		{
			return m_registry;
		}

#pragma endregion

		void process_pending_registry_queries();

	private:
		friend class world_data_provider;

		bool m_shouldStop = false;
		registry m_registry;
		registry_query_queue m_pendingEntityRegistryQueries;
	};
}
