#include <aoe/core/ecs/World.h>


namespace aoe
{
	namespace ecs
	{
		void World::start(sync::MultiThreadSchedule const& a_schedule)
		{
			sync::MultiThreadWorker t_worldWorker{m_tasks, a_schedule};
			while (!m_data.m_shouldStop)
			{
				t_worldWorker.update();
				m_data.update();
			}
			t_worldWorker.stop();
		}
	}
}