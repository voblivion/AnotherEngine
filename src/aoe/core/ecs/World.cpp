#include <vob/aoe/ecs/world.h>

// TODO : remove this, no need to sleep
#include <windows.h>

namespace vob::aoecs
{
	void world::start()
	{
		mismt::worker t_worldWorker{ m_tasks, m_schedule };
		while (!m_data.m_stopManager.should_stop())
		{
			t_worldWorker.execute();
			m_data.update();
            // TODO : remove this, no need to sleep
			Sleep(2);
		}
	}
}