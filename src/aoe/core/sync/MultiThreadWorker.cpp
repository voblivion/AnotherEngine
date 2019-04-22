#include <aoe/core/sync/MultiThreadWorker.h>


namespace aoe
{
	namespace sync
	{
		// Public
		MultiThreadWorker::MultiThreadWorker(TaskList const& a_tasks,
			MultiThreadSchedule const& a_schedule)
			: m_mainWorker{ a_tasks, a_schedule.front() }
		{
			assert(!a_schedule.empty());

			auto const& t_allocator = m_threadWorkers.get_allocator();
			m_threadWorkers.reserve(a_schedule.size() - 1);
			auto t_it = a_schedule.begin();
			while (++t_it != a_schedule.end())
			{
				m_threadWorkers.emplace_back(
					sta::allocatePolymorphic<ThreadWorker>(
						t_allocator, a_tasks, *t_it));
			}
		}

		void MultiThreadWorker::update()
		{
			toThreadWorkers(&ThreadWorker::waitDone);
			toThreadWorkers(&ThreadWorker::askPreUpdate);
			m_mainWorker.preUpdate();

			toThreadWorkers(&ThreadWorker::waitDone);
			toThreadWorkers(&ThreadWorker::askUpdate);
			m_mainWorker.update();
		}

		void MultiThreadWorker::stop()
		{
			toThreadWorkers(&ThreadWorker::waitDone);
			toThreadWorkers(&ThreadWorker::askStop);
		}
	}
}
