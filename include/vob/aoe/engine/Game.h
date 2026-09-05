#pragma once

#include <vob/aoe/api.h>

#include <functional>
#include <memory>
#include <utility>


namespace vob::aoeng
{
	struct IWorld;
	class Application;

	using WorldProvider = std::function<std::shared_ptr<IWorld>(Application&)>;

	struct IGameController
	{
		virtual void requestSwitchWorld(WorldProvider a_worldProvider) = 0;
		virtual void requestStop() = 0;
	};

	struct IWorld
	{
		virtual void start(IGameController& a_gameController) = 0;
		virtual void update() = 0;
		virtual void stop() = 0;
	};

	class Game
	{
	public:
		void run(Application& a_application, std::shared_ptr<IWorld> a_world)
		{
			GameController gameController;
			a_world->start(gameController);

			while (!gameController.hasRequestedStop())
			{
				auto const worldProvider = gameController.acquireLastRequestedWorldProvider();
				if (worldProvider != nullptr)
				{
					a_world->stop();
					a_world = worldProvider(a_application);
					a_world->start(gameController);
				}

				a_world->update();
			}

			a_world->stop();
		}

	private:
		class GameController : public IGameController
		{
		public:
			void requestSwitchWorld(WorldProvider a_worldProvider) override
			{
				m_lastRequestedWorldProvider = std::move(a_worldProvider);
			}

			void requestStop() override
			{
				m_hasRequestedStop = true;
			}

			WorldProvider acquireLastRequestedWorldProvider()
			{
				return std::exchange(m_lastRequestedWorldProvider, nullptr);
			}

			bool hasRequestedStop() const
			{
				return m_hasRequestedStop;
			}

		private:
			WorldProvider m_lastRequestedWorldProvider;
			bool m_hasRequestedStop = false;
		};
	};
}
