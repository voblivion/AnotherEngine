#pragma once

#include <vector>

#include <vob/aoe/core/sync/ATask.h>

namespace vob::aoe::sync
{
	using ThreadSchedule = std::vector<TaskDescription>;

	class VOB_AOE_API Worker
	{
	public:
		// Constructors
		Worker(TaskList const& a_tasks, ThreadSchedule const& a_schedule);

		// Methods
		void preUpdate();

		void update();

	private:
		// Attributes
		std::vector<std::reference_wrapper<ATask>> m_tasks;

		// Methods
		template <typename Functor>
		void toTasks(Functor a_functor)
		{
			for (auto& t_task : m_tasks)
			{
				auto t_res = std::bind(a_functor, &(t_task.get()));
				t_res();
			}
		}
	};
}