#pragma once

#include <vob/misc/multithread/worker.h>

namespace vob::aoeng
{
	class world;
}


void init_world_systems(vob::aoeng::world& a_world, vob::mismt::pmr::schedule& a_schedule);
