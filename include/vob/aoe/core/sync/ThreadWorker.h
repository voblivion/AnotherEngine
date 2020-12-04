#pragma once

#include <condition_variable>
#include <mutex>

#include <vob/aoe/core/sync/ATask.h>
#include <vob/aoe/core/sync/Worker.h>

namespace vob::aoe::sync
{
	class VOB_AOE_API ThreadWorker
		: Worker
	{
		enum class State
		{
			Done,
			PreUpdate,
			Update,
			Stop
		};

	public:
		// Constructors
		ThreadWorker(ThreadWorker&&) = delete;

		ThreadWorker(ThreadWorker const&) = delete;

		ThreadWorker(TaskList const& a_tasks
			, ThreadSchedule const& a_schedule);

		~ThreadWorker();

		// Methods
		void waitDone();

		void askPreUpdate();

		void askUpdate();

		void askStop();

		// Operators
		ThreadWorker& operator=(ThreadWorker&&) = delete;
		ThreadWorker& operator=(ThreadWorker const&) = delete;

	private:
		// Attributes
		State m_state{ State::Done };
		std::mutex m_mutex;
		std::condition_variable m_sync;
		std::thread m_thread;
		std::vector<std::reference_wrapper<ATask>> m_tasks;

		// Methods
		void start();

		void setState(State const a_state);

		State waitForWork();
	};
}
