#pragma once

#include <vector>
#include <aoe/core/ecs/WorldData.h>
#include <aoe/core/ecs/WorldDataProvider.h>
#include <aoe/core/sync/ATask.h>
#include <aoe/core/sync/MultiThreadWorker.h>

namespace aoe
{
	namespace ecs
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
			explicit World(sta::Allocator<std::byte> const& a_allocator)
				: m_data{ a_allocator }
				, m_tasks{ a_allocator }
			{}

			// Methods
			template <typename SystemType>
			std::size_t addSystem()
			{
				auto const& t_allocator = m_tasks.get_allocator();
				m_tasks.emplace_back(sta::allocatePolymorphic<
					detail::SystemTask<SystemType>>(t_allocator, m_data));
				return m_tasks.size() - 1;
			}

			AOE_CORE_API void start(
				sync::MultiThreadSchedule const& a_schedule);

			WorldData& getData()
			{
				return m_data;
			}

		private:
			// Attributes
			WorldData m_data;
			sync::TaskList m_tasks;
		};
	}
}
