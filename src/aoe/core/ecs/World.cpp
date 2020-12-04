#include <vob/aoe/core/ecs/World.h>

namespace vob::aoe::ecs
{
	void World::start()
	{
		sync::MultiThreadWorker t_worldWorker{m_tasks, m_schedule};
		while (!m_data.m_shouldStop)
		{
			t_worldWorker.update();
			m_data.update();
		}
		t_worldWorker.stop();
	}

	void World::step()
	{
	}
}