#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/engine/FrameJob.h>

#include <atomic>
#include <memory>
#include <thread>
#include <vector>


namespace vob::aoeng
{
	class VOB_AOE_API UnsynchronizedFrameJobsRepeater
	{
	public:
		explicit UnsynchronizedFrameJobsRepeater(std::vector<std::shared_ptr<IFrameJob>> a_frameJobs);

		UnsynchronizedFrameJobsRepeater(UnsynchronizedFrameJobsRepeater const&) = delete;
		UnsynchronizedFrameJobsRepeater& operator=(UnsynchronizedFrameJobsRepeater const&) = delete;

		void prepare();

		void execute();

		void cleanup();

	private:
		class Worker
		{
		public:
			explicit Worker(std::shared_ptr<IFrameJob> a_frameJob);

			void start();

			void requestStop();

			void waitStopDone();

		private:
			std::shared_ptr<IFrameJob> m_frameJob;

			std::atomic<bool> m_shouldRun = false;
			std::thread m_thread;

			void run();
		};

		std::shared_ptr<IFrameJob> m_mainFrameJob;
		std::vector<std::unique_ptr<Worker>> m_workers;
	};
}
