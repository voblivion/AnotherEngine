#include <aoe/core/sync/Worker.h>


namespace aoe
{
	namespace sync
	{
		// Public
		Worker::Worker(TaskList const& a_tasks, ThreadSchedule const& a_schedule)
		{
			m_tasks.reserve(a_schedule.size());
			for (auto const& t_taskDescription : a_schedule)
			{
				auto& t_task = m_tasks.emplace_back(
					*a_tasks[t_taskDescription.m_id]);
				for (auto t_dependency : t_taskDescription.m_dependencies)
				{
					t_task.get().addDependency(*a_tasks[t_dependency]);
				}
			}
		}

		void Worker::preUpdate()
		{
			toTasks(&ATask::preUpdate);
		}

		void Worker::update()
		{
			toTasks(&ATask::update);
		}
	}
}
