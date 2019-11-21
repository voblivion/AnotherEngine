#include <vob/aoe/core/ecs/World.h>

namespace vob::aoe::ecs
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