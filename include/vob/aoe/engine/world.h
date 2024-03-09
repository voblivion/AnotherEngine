#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/engine/world_data.h>
#include <vob/aoe/engine/world_data_provider.h>

#include <vob/misc/multithread/basic_task.h>
#include <vob/misc/multithread/worker.h>
#include <vob/misc/std/message_macros.h>


namespace vob::aoeng
{
	namespace detail
	{
		template <typename TSystem>
		class system_task final : public mismt::basic_task
		{
		public:
			explicit system_task(world_data& a_worldData)
				: m_worldDataProvider{ a_worldData }
				, m_system{ m_worldDataProvider }
			{
			}

			virtual void execute() const override
			{
				m_system.update();
			}

		private:
#pragma message(VOB_MISTD_TODO "use collected accesses for safe multithreading.")
			world_data_provider m_worldDataProvider;
			TSystem m_system;
		};
	}

	using system_id = mismt::task_id;
	using system_allocator = std::pmr::polymorphic_allocator<std::shared_ptr<mismt::basic_task>>;

	class VOB_AOE_API world
	{
	public:
		explicit world(
			registry_allocator const& a_registryAllocator = {},
			registry_query_allocator const& a_entityRegistryQueryAllocator = {},
			system_allocator const& a_systemAllocator = {}
		);

		world_data& get_data()
		{
			return m_worldData;
		}

		template <typename TWorldComponent, typename... TArgs>
		decltype(auto) add_world_component(TArgs&&... a_args)
		{
			return m_worldData.add_world_component<TWorldComponent>(std::forward<TArgs>(a_args)...);
		}

		template <typename TWorldComponent>
		decltype(auto) get_world_component()
		{
			return m_worldData.get_world_component<TWorldComponent>();
		}

		template <typename TSystem>
		system_id add_system()
		{
			m_systemTasks.emplace_back(std::allocate_shared<detail::system_task<TSystem>>(
				m_systemTasks.get_allocator(), m_worldData));
			return m_systemTasks.size() - 1;
		}

		void start(mismt::pmr::schedule a_schedule);

	private:
		world_data m_worldData;

		using system_list = std::vector<std::shared_ptr<mismt::basic_task>, system_allocator>;
		system_list m_systemTasks;
	};
}
