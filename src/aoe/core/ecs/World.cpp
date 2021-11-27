#include <vob/aoe/ecs/World.h>

// TODO : remove this, no need to sleep
#include <windows.h>

namespace vob::aoecs
{
	void World::start()
	{
		aoe::sync::MultiThreadWorker t_worldWorker{m_tasks, m_schedule};
		while (!m_data.m_shouldStop)
		{
			t_worldWorker.update();
			m_data.update();
            // TODO : remove this, no need to sleep
			Sleep(2);
		}
		t_worldWorker.stop();
	}

	void World::step()
	{
	}
}