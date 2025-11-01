#include <vob/aoe/engine/UnsynchronizedFrameJobsRepeater.h>

#include <cassert>
#include <ranges>


namespace vob::aoeng
{
	UnsynchronizedFrameJobsRepeater::UnsynchronizedFrameJobsRepeater(std::vector<std::shared_ptr<IFrameJob>> a_frameJobs)
	{
		assert(!a_frameJobs.empty());
		m_mainFrameJob = a_frameJobs.front();
		m_workers.reserve(a_frameJobs.size());
		for (auto const& frameJob : a_frameJobs | std::views::drop(1))
		{
			m_workers.push_back(std::make_unique<Worker>(frameJob));
		}
	}

	void UnsynchronizedFrameJobsRepeater::prepare()
	{
		for (auto& worker : m_workers)
		{
			worker->start();
		}

		m_mainFrameJob->prepare();
	}

	void UnsynchronizedFrameJobsRepeater::execute()
	{
		m_mainFrameJob->execute();
	}

	void UnsynchronizedFrameJobsRepeater::cleanup()
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

	UnsynchronizedFrameJobsRepeater::Worker::Worker(std::shared_ptr<IFrameJob> a_frameSchedule)
		: m_frameJob{ std::move(a_frameSchedule) }
	{
	}

	void UnsynchronizedFrameJobsRepeater::Worker::start()
	{
		m_shouldRun.store(true);
		m_thread = std::thread{ &Worker::run, this };
	}

	void UnsynchronizedFrameJobsRepeater::Worker::requestStop()
	{
		m_shouldRun.store(false);
	}

	void UnsynchronizedFrameJobsRepeater::Worker::waitStopDone()
	{
		m_thread.join();
	}

	void UnsynchronizedFrameJobsRepeater::Worker::run()
	{
		m_frameJob->prepare();

		while (m_shouldRun.load())
		{
			m_frameJob->execute();
		}

		m_frameJob->cleanup();
	}
}
