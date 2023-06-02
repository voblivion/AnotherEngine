#pragma once

#include <vob/aoe/api.h>

#include <vob/misc/std/message_macros.h>

#include <entt/entt.hpp>


namespace vob::aoeng
{
	using entity = entt::entity;
	using entity_allocator = std::allocator<entity>; // std::pmr::polymorphic_allocator<entity>;
	using entity_registry = entt::basic_registry<entt::entity, entity_allocator>;
	using entity_registry_query = std::function<void(entity_registry&)>;
	using entity_registry_query_queue = std::pmr::vector<entity_registry_query>;
	using entity_registry_query_allocator = std::pmr::polymorphic_allocator<entity_registry_query>;

	class VOB_AOE_API world_data
	{
	public:
		explicit world_data(
			entity_allocator const& a_entityAllocator = {},
			entity_registry_query_allocator const& a_entityRegistryQueryAllocator = {}
		);

		template <typename TComponent, typename... TArgs>
		decltype(auto) add_world_component(TArgs&&... a_args)
		{
			return m_entityRegistry.emplace<TComponent>(m_worldEntity, std::forward<TArgs>(a_args)...);
		}

		bool should_stop() const
		{
			return m_shouldStop;
		}

#pragma message(VOB_MISTD_TODO "those functions are needed for world init, but need a better pattern.")
#pragma region TMP
		entity_registry& get_entity_registry()
		{
			return m_entityRegistry;
		}

		entity get_world_entity()
		{
			return m_worldEntity;
		}
#pragma endregion

		void process_pending_registry_queries();

	private:
		friend class world_data_provider;

		bool m_shouldStop = false;
		entity_registry m_entityRegistry;
		entity m_worldEntity;
		entity_registry_query_queue m_pendingEntityRegistryQueries;
	};
}
