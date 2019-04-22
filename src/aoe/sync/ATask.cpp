#include <aoe/sync/ATask.h>

namespace aoe
{
	namespace sync
	{
		// Public
		void ATask::addDependency(ATask& a_task)
		{
			m_dependencies.emplace_back(a_task);
		}

		void ATask::preUpdate()
		{
			setState(State::Pending);
		}

		void ATask::update()
		{
			for (auto& t_dependency : m_dependencies)
			{
				t_dependency.get().waitDone();
			}
			doUpdate();
			setState(State::Done);
		}

		void ATask::waitDone()
		{
			std::unique_lock<std::mutex> t_lock{ m_mutex };
			m_sync.wait(t_lock, [this]
			{
				return m_state == State::Done;
			});
		}

		// Private
		void ATask::setState(State const a_state)
		{
			{
				std::lock_guard<std::mutex> t_lock{ m_mutex };
				m_state = a_state;
			}
			m_sync.notify_all();
		}
	}
}
