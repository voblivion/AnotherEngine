#pragma once

#include <vob/aoe/api.h>

#include <memory>


namespace vob::aoeng
{
	struct IWorld;

	struct IGameController
	{
		virtual void requestSwitchWorld(std::shared_ptr<IWorld> a_newWorld) = 0;
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
		void run(std::shared_ptr<IWorld> a_world)
		{
			GameController gameController;
			a_world->start(gameController);

			while (!gameController.hasRequestedStop())
			{
				std::shared_ptr<IWorld> lastRequestedWorld = gameController.acquireLastRequestedWorld();
				if (lastRequestedWorld != nullptr)
				{
					a_world->stop();
					a_world = std::move(lastRequestedWorld);
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
			void requestSwitchWorld(std::shared_ptr<IWorld> a_newWorld) override
			{
				m_lastRequestedWorld = a_newWorld;
			}

			void requestStop() override
			{
				m_hasRequestedStop = true;
			}

			std::shared_ptr<IWorld> acquireLastRequestedWorld()
			{
				return std::move(m_lastRequestedWorld);
			}

			bool hasRequestedStop() const
			{
				return m_hasRequestedStop;
			}

		private:
			std::shared_ptr<IWorld> m_lastRequestedWorld;
			bool m_hasRequestedStop = false;
		};
	};
}