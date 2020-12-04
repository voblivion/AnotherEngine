#pragma once

#include <vector>

#include <vob/aoe/core/ecs/WorldData.h>
#include <vob/aoe/core/ecs/WorldDataProvider.h>
#include <vob/aoe/core/ecs/ComponentManager.h>
#include <vob/aoe/core/sync/ATask.h>
#include <vob/aoe/core/sync/MultiThreadWorker.h>

namespace vob::aoe::ecs
{
	namespace detail
	{
		template <typename SystemType>
		class SystemTask final
			: public sync::ATask
		{
		public:
			// Constructors
			explicit SystemTask(WorldData& a_worldData)
				: m_worldDataProvider{ a_worldData }
				, m_system{ m_worldDataProvider }
			{}

		protected:
			// Methods
			virtual void doUpdate() override
			{
				m_system.update();
			}

		private:
			// Attributes
			WorldDataProvider m_worldDataProvider;
			SystemType m_system;
		};
	}

	class World
	{
	public:
		// Constructors
		explicit World(ComponentManager a_worldComponents)
			: m_data{ std::move(a_worldComponents) }
		{}

		// Methods
		template <typename SystemType>
		std::size_t addSystem()
		{
			using Task = detail::SystemTask<SystemType>;
			m_tasks.emplace_back(std::make_unique<Task>(m_data));
			return m_tasks.size() - 1;
		}

		void setSchedule(sync::MultiThreadSchedule a_schedule)
		{
			m_schedule = std::move(a_schedule);
		}
		VOB_AOE_API void start();
		VOB_AOE_API void step();

		WorldData& getData()
		{
			return m_data;
		}

	private:
		// Attributes
		sync::TaskList m_tasks;
		WorldData m_data;
		sync::MultiThreadSchedule m_schedule;
	};
}
