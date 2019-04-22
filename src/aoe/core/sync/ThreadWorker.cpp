#include <aoe/core/sync/ThreadWorker.h>


namespace aoe
{
	namespace sync
	{
		// Public
		ThreadWorker::ThreadWorker(TaskList const& a_tasks,
			ThreadSchedule const& a_schedule)
			: Worker{ a_tasks, a_schedule }
		{
			m_thread = std::thread{ &ThreadWorker::start, this };
		}

		ThreadWorker::~ThreadWorker()
		{
			m_thread.join();
		}

		void ThreadWorker::waitDone()
		{
			std::unique_lock<std::mutex> t_lock{ m_mutex };
			m_sync.wait(t_lock, [this]
			{
				return m_state == State::Done;
			});
		}

		void ThreadWorker::askPreUpdate()
		{
			setState(State::PreUpdate);
		}

		void ThreadWorker::askUpdate()
		{
			setState(State::Update);
		}

		void ThreadWorker::askStop()
		{
			setState(State::Stop);
		}

		// Private
		void ThreadWorker::start()
		{
			auto t_state = State::Done;
			while (t_state != State::Stop)
			{
				t_state = waitForWork();
				switch (t_state)
				{
				case State::PreUpdate:
					preUpdate();
					setState(State::Done);
					break;
				case State::Update:
					update();
					setState(State::Done);
					break;
				default:
					break;
				}
			}
		}

		void ThreadWorker::setState(State const a_state)
		{
			{
				std::lock_guard<std::mutex> t_lock{ m_mutex };
				m_state = a_state;
			}
			m_sync.notify_all();
		}

		ThreadWorker::State ThreadWorker::waitForWork()
		{
			std::unique_lock<std::mutex> t_lock{ m_mutex };
			m_sync.wait(t_lock, [this]
			{
				return m_state != State::Done;
			});
			return m_state;
		}
	}
}
