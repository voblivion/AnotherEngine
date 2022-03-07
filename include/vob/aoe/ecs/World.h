#pragma once

#include <vob/aoe/ecs/world_data.h>
#include <vob/aoe/ecs/WorldDataProvider.h>
#include <vob/aoe/ecs/component_manager.h>
#include <vob/misc/multithread/basic_task.h>
#include <vob/misc/multithread/worker.h>

#include <vector>


namespace vob::aoecs
{
	namespace detail
	{
		template <typename SystemType>
		class system_task final
			: public mismt::basic_task
		{
		public:
			// Constructors
			explicit system_task(world_data& a_worldData)
				: m_worldDataProvider{ a_worldData }
				, m_system{ m_worldDataProvider }
			{}

		protected:
			// Methods
			virtual void execute() const override
			{
				m_system.update();
			}

		private:
			// Attributes
			WorldDataProvider m_worldDataProvider;
			SystemType m_system;
		};
	}

	class world
	{
	public:
		// Constructors
		explicit world(component_manager a_worldComponents)
			: m_data{ std::move(a_worldComponents) }
		{}

		// Methods
		template <typename SystemType>
		std::size_t add_system()
		{
			m_tasks.emplace_back(std::make_unique<detail::system_task<SystemType>>(m_data));
			return m_tasks.size() - 1;
		}

		void set_schedule(mismt::schedule a_schedule)
		{
			m_schedule = std::move(a_schedule);
		}

		VOB_AOE_API void start();

		world_data& get_data()
		{
			return m_data;
		}

	private:
		// Attributes
		mismt::task_list m_tasks;
		world_data m_data;
		mismt::schedule m_schedule;
	};
}
