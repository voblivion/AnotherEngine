#pragma once

namespace vob::aoeng
{
	class world;
}

namespace vob::aoewi
{
	class glfw_window;
}

namespace vob::aoe
{
	struct DataHolder;
}


void init_world_components(
	vob::aoeng::world& a_world,
	vob::aoe::DataHolder& a_data,
	vob::aoewi::glfw_window& a_window);
