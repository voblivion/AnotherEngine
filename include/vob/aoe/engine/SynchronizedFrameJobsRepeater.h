#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/engine/frame_job.h>

#include <cassert>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <ranges>
#include <span>
#include <vector>


namespace vob::aoeng
{
	class VOB_AOE_API SynchronizedFrameJobsRepeater
	{
	public:
		template <typename TFrameJobs>
		explicit SynchronizedFrameJobsRepeater(TFrameJobs a_frameJobs)
		{
			assert(!a_frameJobs.empty());
			m_mainFrameJob = std::move(a_frameJobs.front());
			m_workers.reserve(a_frameJobs.size());
			for (auto& frameJob : a_frameJobs | std::views::drop(1))
			{
				m_workers.push_back(std::make_unique<Worker>(std::move(frameJob)));
			}
		}

		SynchronizedFrameJobsRepeater(SynchronizedFrameJobsRepeater const&) = delete;
		SynchronizedFrameJobsRepeater& operator=(SynchronizedFrameJobsRepeater const&) = delete;

		void prepare();

		void execute();

		void cleanup();

	private:
		class Worker
		{
		public:
			explicit Worker(std::shared_ptr<IFrameJob> a_frameSchedule);

			void requestStart();

			void waitStartDone();

			void requestExecute();

			void waitExecuteDone();

			void requestStop();

			void waitStopDone();


		private:
			void run();

			enum class State
			{
				Idle,
				Executing,
				Stopped
			};

			std::shared_ptr<IFrameJob> m_frameJob;
			State m_state = State::Idle;
			std::mutex m_mutex;
			std::condition_variable m_cv;
			std::thread m_thread;
		};

		std::shared_ptr<IFrameJob> m_mainFrameJob;
		std::vector<std::unique_ptr<Worker>> m_workers;
	};
}
