#pragma once

#include <vob/sta/memory.h>
#include <vob/aoe/core/sync/ATask.h>
#include <vob/aoe/core/sync/ThreadWorker.h>
#include <vob/aoe/core/sync/Worker.h>

namespace vob::aoe::sync
{
	using MultiThreadSchedule = std::vector<ThreadSchedule>;

	class VOB_AOE_API MultiThreadWorker
	{
	public:
		// Constructors
		MultiThreadWorker(MultiThreadWorker&&) = default;

		MultiThreadWorker(MultiThreadWorker const&) = delete;

		MultiThreadWorker(TaskList const& a_tasks
			, MultiThreadSchedule const& a_schedule);

		~MultiThreadWorker() = default;

		// Methods
		void update();

		void stop();

		// Operators
		MultiThreadWorker& operator=(MultiThreadWorker&&) = default;

		MultiThreadWorker& operator=(MultiThreadWorker const&) = delete;

	private:
		// Attributes
		Worker m_mainWorker;
		std::pmr::vector<sta::polymorphic_ptr<ThreadWorker>> m_threadWorkers;

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
