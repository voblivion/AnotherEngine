#pragma once

#include <aoe/core/standard/Memory.h>
#include <aoe/core/sync/ATask.h>
#include <aoe/core/sync/ThreadWorker.h>
#include <aoe/core/sync/Worker.h>

namespace aoe
{
	namespace sync
	{
		using MultiThreadSchedule = std::vector<ThreadSchedule>;

		class AOE_CORE_API MultiThreadWorker
		{
		public:
			// Constructors
			MultiThreadWorker(TaskList const& a_tasks
				, MultiThreadSchedule const& a_schedule);
			MultiThreadWorker(MultiThreadWorker&&) = default;

			// Methods
			void update();

			void stop();

			MultiThreadWorker& operator=(MultiThreadWorker&&) = default;

		private:
			// Attributes
			Worker m_mainWorker;
			std::pmr::vector<sta::PolymorphicPtr<ThreadWorker>> m_threadWorkers;

			// Methods
			template <typename Functor>
			void toThreadWorkers(Functor a_functor)
			{
				for (auto& t_threadWorker : m_threadWorkers)
				{
					auto t_res = std::bind(a_functor, t_threadWorker.get());
					t_res();
				}
			}
		};
	}
}
