#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/engine/game.h>
#include <vob/aoe/engine/frame_job.h>
#include <vob/aoe/engine/UnsynchronizedFrameJobsRepeater.h>

#include <memory>
#include <mutex>
#include <vector>


namespace vob::aoeng
{
	class MultiWorld : public IWorld
	{
	public:
		MultiWorld(std::vector<std::shared_ptr<IWorld>> const& a_worlds)
			: m_threadSafeGameController{ std::make_unique<ThreadSafeGameController>() }
			, m_unsynchronizedFrameScheduler{ makeFrameSchedules(a_worlds, *m_threadSafeGameController) }
		{
		}

		void start(IGameController& a_gameController) override
		{
			m_threadSafeGameController->setUnderlyingGameController(&a_gameController);

			m_unsynchronizedFrameScheduler.prepare();

			m_threadSafeGameController->lockUnderlyingGameController();
		}

		void update() override
		{
			m_threadSafeGameController->unlockUnderlyingGameController();

			m_unsynchronizedFrameScheduler.execute();

			m_threadSafeGameController->lockUnderlyingGameController();
		}

		void stop() override
		{
			m_threadSafeGameController->unlockUnderlyingGameController();

			m_unsynchronizedFrameScheduler.cleanup();

			m_threadSafeGameController->setUnderlyingGameController(nullptr);
		}

	private:
		class ThreadSafeGameController : public IGameController
		{
		public:
			void setUnderlyingGameController(IGameController* a_gameController)
			{
				m_gameController = a_gameController;
			}

			void lockUnderlyingGameController()
			{
				m_mutex.lock();
			}

			void unlockUnderlyingGameController()
			{
				m_mutex.unlock();
			}

			void requestSwitchWorld(std::shared_ptr<IWorld> a_newWorld) override
			{
				std::lock_guard lock(m_mutex);
				assert(m_gameController != nullptr);
				m_gameController->requestSwitchWorld(std::move(a_newWorld));
			}

			void requestStop() override
			{
				std::lock_guard lock(m_mutex);
				assert(m_gameController != nullptr);
				m_gameController->requestStop();
			}

		private:
			std::mutex m_mutex;
			IGameController* m_gameController = nullptr;
		};

		std::vector<std::shared_ptr<IFrameJob>> makeFrameSchedules(
			std::vector<std::shared_ptr<IWorld>> const& a_worlds, ThreadSafeGameController& a_gameController)
		{
			std::vector<std::shared_ptr<IFrameJob>> frameSchedules;
			frameSchedules.reserve(a_worlds.size());
			for (auto const& world : a_worlds)
			{
				frameSchedules.push_back(std::make_shared<FrameJob>(world, a_gameController));
			}
			return frameSchedules;
		}

		class FrameJob final : public IFrameJob
		{
		public:
			explicit FrameJob(std::shared_ptr<IWorld> a_world, ThreadSafeGameController& a_gameController)
				: m_world{ std::move(a_world) }
				, m_gameController{ a_gameController }
			{
			}

			void prepare() override
			{
				m_world->start(m_gameController.get());
			}

			void execute() override
			{
				m_world->update();
			}

			void cleanup() override
			{
				m_world->stop();
			}

			std::shared_ptr<IWorld> m_world;
			std::reference_wrapper<ThreadSafeGameController> m_gameController;
		};

		std::unique_ptr<ThreadSafeGameController> m_threadSafeGameController;
		UnsynchronizedFrameJobsRepeater m_unsynchronizedFrameScheduler;
	};
}
