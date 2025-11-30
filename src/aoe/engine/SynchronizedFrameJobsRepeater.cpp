#include <vob/aoe/engine/SynchronizedFrameJobsRepeater.h>

#include "optick.h"

#include <ranges>


namespace vob::aoeng
{
	void SynchronizedFrameJobsRepeater::prepare()
	{
		for (auto& worker : m_workers)
		{
			worker->requestStart();
		}

		m_mainFrameJob->prepare();

		for (auto& worker : m_workers)
		{
			worker->waitStartDone();
		}
	}

	void SynchronizedFrameJobsRepeater::execute()
	{
		for (auto& worker : m_workers)
		{
			worker->requestExecute();
		}

		m_mainFrameJob->execute();

		for (auto& worker : m_workers)
		{
			worker->waitExecuteDone();
		}
	}

	void SynchronizedFrameJobsRepeater::cleanup()
	{
		for (auto& worker : m_workers)
		{
			worker->requestStop();
		}

		m_mainFrameJob->cleanup();

		for (auto& worker : m_workers)
		{
			worker->waitStopDone();
		}
	}

	SynchronizedFrameJobsRepeater::Worker::Worker(std::shared_ptr<IFrameJob> a_frameJob)
		: m_frameJob{ std::move(a_frameJob) }
	{
	}

	void SynchronizedFrameJobsRepeater::Worker::requestStart()
	{
		m_state = State::Stopped;
		m_thread = std::thread{ &Worker::run, this };
	}

	void SynchronizedFrameJobsRepeater::Worker::waitStartDone()
	{
		std::unique_lock lock(m_mutex);
		m_cv.wait(lock, [this] { return m_state == State::Idle; });
	}

	void SynchronizedFrameJobsRepeater::Worker::requestExecute()
	{
		std::lock_guard lock(m_mutex);
		m_state = State::Executing;
		m_cv.notify_one();
	}

	void SynchronizedFrameJobsRepeater::Worker::waitExecuteDone()
	{
		std::unique_lock lock(m_mutex);
		m_cv.wait(lock, [this] { return m_state == State::Idle; });
	}

	void SynchronizedFrameJobsRepeater::Worker::requestStop()
	{
		std::lock_guard lock(m_mutex);
		m_state = State::Stopped;
		m_cv.notify_one();
	}

	void SynchronizedFrameJobsRepeater::Worker::waitStopDone()
	{
		m_thread.join();
	}

	void SynchronizedFrameJobsRepeater::Worker::run()
	{
		m_frameJob->prepare();

		std::unique_lock lock(m_mutex);
		m_state = State::Idle;
		m_cv.notify_one();
		m_cv.wait(lock, [this] { return m_state != State::Idle; });

		while (m_state == State::Executing)
		{
			lock.unlock();
			m_frameJob->execute();
			lock.lock();

			m_state = State::Idle;
			m_cv.notify_one();
			m_cv.wait(lock, [this] { return m_state != State::Idle; });
		}

		lock.unlock();
		m_frameJob->cleanup();
		lock.lock();
		m_state = State::Idle;
		m_cv.notify_one();
	}
}
