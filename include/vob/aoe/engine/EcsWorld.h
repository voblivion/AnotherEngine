#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include <vob/aoe/engine/game.h>
#include <vob/aoe/engine/SynchronizedFrameJobsRepeater.h>

#include <tracy/Tracy.hpp>

#include <memory>
#include <vector>


namespace vob::aoeng
{
	using EcsSystemId = int32_t;

	struct IEcsSystem
	{
		virtual ~IEcsSystem() = default;

		virtual void init(EcsWorldDataAccessRegistrar& a_wdar) = 0;
		virtual void execute(EcsWorldDataAccessProvider const& a_wdap) const = 0;
	};

	template <typename TSystem>
	class BasicEcsSystem : public IEcsSystem
	{
	public:
		BasicEcsSystem(TSystem a_system = {})
			: m_system{ std::move(a_system) }
		{
		}

		void init(EcsWorldDataAccessRegistrar& a_wdar) override
		{
			m_system.init(a_wdar);
		}

		void execute(EcsWorldDataAccessProvider const& a_wdap) const override
		{
			static const tracy::SourceLocationData loc{
				std::string_view{ typeid(TSystem).name() }.substr(std::string_view{typeid(TSystem).name()}.rfind(':') + 1).data(),
				TracyFunction,
				TracyFile,
				TracyLine,
				static_cast<uint32_t>(reinterpret_cast<intptr_t>(typeid(TSystem).name())) /* color */
			};
			tracy::ScopedZone varname(&loc, TRACY_CALLSTACK, true /* active */);
			m_system.execute(a_wdap);
		}

	private:
		TSystem m_system;
	};

	struct EcsSystemSchedule
	{
		EcsSystemId id;
		std::vector<EcsSystemId> dependencies;
	};

	struct EcsThreadSchedule
	{
		std::string name;
		std::vector<EcsSystemSchedule> systems;
	};

	using EcsSchedule = std::vector<EcsThreadSchedule>;

	class VOB_AOE_API EcsWorld : public IWorld
	{
	public:
		EcsWorld(std::vector<std::shared_ptr<IEcsSystem>> a_systems, EcsSchedule a_scheduleInfo, entt::registry a_registry = {});

		EcsWorld(EcsWorld const&) = delete;
		EcsWorld& operator=(EcsWorld const&) = delete;
		~EcsWorld() = default;

		void start(IGameController& a_gameController) override
		{
			m_wdp = std::make_unique<EcsWorldDataAccessProvider>(EcsWorldDataAccessProvider(m_registry, m_queryQueue, a_gameController));
			m_synchronizedFrameScheduler.prepare();
		}

		void update() override
		{
			m_synchronizedFrameScheduler.execute();

			for (auto& query : m_queryQueue)
			{
				query(m_registry);
			}
			m_queryQueue.clear();
		}

		void stop() override
		{
			m_synchronizedFrameScheduler.cleanup();
			m_wdp = nullptr;
		}

	private:
		class EcsSystemState
		{
		public:
			void setIsDone(bool a_isDone)
			{
				std::lock_guard lock(m_mutex);
				m_isDone = a_isDone;
				m_cv.notify_all();
			}

			void waitUntilDone()
			{
				std::unique_lock lock(m_mutex);
				m_cv.wait(lock, [this] { return m_isDone; });
			}

		private:
			bool m_isDone = true;
			std::mutex m_mutex;
			std::condition_variable m_cv;
		};

		struct EcsFrameJob : public IFrameJob
		{
			enum class State
			{
				Reset,
				Execute
			};

			explicit EcsFrameJob(EcsWorld* a_ecsWorld, EcsThreadSchedule a_threadSchedule)
				: m_ecsWorld{ std::move(a_ecsWorld) }
				, m_threadSchedule{ std::move(a_threadSchedule) }
			{
			}

			void requestState(State a_state)
			{
				std::lock_guard lock(m_mutex);
				m_requestedState = a_state;
				m_cv.notify_one();
			}

			void waitStateCompleted(State a_state)
			{
				std::unique_lock lock(m_mutex);
				m_cv.wait(lock, [this, a_state] { return m_completedState == a_state; });
			}

			void prepare() override
			{
				tracy::SetThreadName(m_threadSchedule.name.c_str());
			}

			void execute() override
			{
				std::unique_lock lock(m_mutex);
				m_cv.wait(lock, [this] { return m_requestedState == State::Reset; });

				doReset();

				lock.lock();
				m_completedState = State::Reset;
				m_cv.notify_one();
				lock.unlock();

				m_cv.wait(lock, [this] { return m_requestedState == State::Execute; });

				doExecute();

				lock.lock();
				m_completedState = State::Execute;
				m_cv.notify_one();
			}

			void cleanup() override
			{

			}

		protected:
			EcsWorld* m_ecsWorld = nullptr;
			EcsThreadSchedule m_threadSchedule;

			State m_requestedState = State::Execute;
			State m_completedState = State::Reset;
			std::mutex m_mutex;
			std::condition_variable m_cv;

			void doReset()
			{
				for (auto& systemSchedule : m_threadSchedule.systems)
				{
					m_ecsWorld->m_systemStates[systemSchedule.id]->setIsDone(false);
				}
			}

			void doExecute()
			{
				for (auto& systemSchedule : m_threadSchedule.systems)
				{
					for (auto dependency : systemSchedule.dependencies)
					{
						m_ecsWorld->m_systemStates[dependency]->waitUntilDone();
					}

					m_ecsWorld->m_systems[systemSchedule.id]->execute(*m_ecsWorld->m_wdp);
					m_ecsWorld->m_systemStates[systemSchedule.id]->setIsDone(true);
				}
			}
		};

		struct EcsMainFrameJob final : public EcsFrameJob
		{
			explicit EcsMainFrameJob(EcsWorld* a_ecsWorld, EcsThreadSchedule a_threadSchedule)
				: EcsFrameJob(a_ecsWorld, a_threadSchedule)
			{
			}

			void execute() override
			{
				for (auto& frameJob : m_ecsWorld->m_frameJobs | std::views::drop(1))
				{
					frameJob->requestState(EcsFrameJob::State::Reset);
				}

				doReset();

				for (auto& frameJob : m_ecsWorld->m_frameJobs | std::views::drop(1))
				{
					frameJob->waitStateCompleted(EcsFrameJob::State::Reset);
				}

				for (auto& frameJob : m_ecsWorld->m_frameJobs | std::views::drop(1))
				{
					frameJob->requestState(EcsFrameJob::State::Execute);
				}

				doExecute();

				for (auto& frameJob : m_ecsWorld->m_frameJobs | std::views::drop(1))
				{
					frameJob->waitStateCompleted(EcsFrameJob::State::Execute);
				}
			}
		};

		std::vector<std::shared_ptr<EcsFrameJob>> makeFrameJobs(EcsSchedule a_schedule)
		{
			std::vector<std::shared_ptr<EcsFrameJob>> frameJobs;
			frameJobs.reserve(a_schedule.size());
			frameJobs.push_back(std::make_shared<EcsMainFrameJob>(this, std::move(a_schedule.front())));
			for (auto& threadSchedule : a_schedule | std::views::drop(1))
			{
				frameJobs.push_back(std::make_shared<EcsFrameJob>(this, std::move(threadSchedule)));
			}

			return frameJobs;
		}

		entt::registry m_registry;
		std::vector<std::shared_ptr<IEcsSystem>> m_systems;
		std::pmr::vector<EcsWorldQuery> m_queryQueue;

		std::vector<std::shared_ptr<EcsFrameJob>> m_frameJobs;
		std::vector<std::unique_ptr<EcsSystemState>> m_systemStates;
		SynchronizedFrameJobsRepeater m_synchronizedFrameScheduler;
		std::unique_ptr<EcsWorldDataAccessProvider> m_wdp = nullptr;
	};
}
